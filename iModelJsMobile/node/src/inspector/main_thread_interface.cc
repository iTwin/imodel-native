#include "main_thread_interface.h"

#include "node_mutex.h"
#include "v8-inspector.h"

#include <functional>
#include <unicode/unistr.h>

namespace node {
namespace inspector {
namespace {

using v8_inspector::StringView;
using v8_inspector::StringBuffer;

template <typename T>
class DeletableWrapper : public Deletable {
 public:
  explicit DeletableWrapper(std::unique_ptr<T> object)
                        : object_(std::move(object)) {}
  ~DeletableWrapper() override = default;

  static T* get(MainThreadInterface* thread, int id) {
    return
        static_cast<DeletableWrapper<T>*>(thread->GetObject(id))->object_.get();
  }

 private:
  std::unique_ptr<T> object_;
};

template <typename T>
std::unique_ptr<Deletable> WrapInDeletable(std::unique_ptr<T> object) {
  return std::unique_ptr<DeletableWrapper<T>>(
      new DeletableWrapper<T>(std::move(object)));
}

template <typename Factory>
class CreateObjectRequest : public Request {
 public:
  CreateObjectRequest(int object_id, Factory factory)
                      : object_id_(object_id), factory_(std::move(factory)) {}

  void Call(MainThreadInterface* thread) {
    thread->AddObject(object_id_, WrapInDeletable(factory_(thread)));
  }

 private:
  int object_id_;
  Factory factory_;
};

template <typename Factory>
std::unique_ptr<Request> NewCreateRequest(int object_id, Factory factory) {
  return std::unique_ptr<Request>(
      new CreateObjectRequest<Factory>(object_id, std::move(factory)));
}

class DeleteRequest : public Request {
 public:
  explicit DeleteRequest(int object_id) : object_id_(object_id) {}

  void Call(MainThreadInterface* thread) override {
    thread->RemoveObject(object_id_);
  }

 private:
  int object_id_;
};

template <typename Target, typename Fn>
class CallRequest : public Request {
 public:
  CallRequest(int id, Fn fn) : id_(id), fn_(std::move(fn)) {}

  void Call(MainThreadInterface* thread) override {
    fn_(DeletableWrapper<Target>::get(thread, id_));
  }

 private:
  int id_;
  Fn fn_;
};

class DispatchMessagesTask : public v8::Task {
 public:
  explicit DispatchMessagesTask(MainThreadInterface* thread)
                                : thread_(thread) {}

  void Run() override {
    thread_->DispatchMessages();
  }

 private:
  MainThreadInterface* thread_;
};

void DisposePairCallback(uv_handle_t* ref) {
  using AsyncAndInterface = std::pair<uv_async_t, MainThreadInterface*>;
  AsyncAndInterface* pair = node::ContainerOf(
      &AsyncAndInterface::first, reinterpret_cast<uv_async_t*>(ref));
  delete pair;
}

template <typename T>
class AnotherThreadObjectReference {
 public:
  AnotherThreadObjectReference(
      std::shared_ptr<MainThreadHandle> thread, int object_id)
      : thread_(thread), object_id_(object_id) {}

  template <typename Factory>
  AnotherThreadObjectReference(
      std::shared_ptr<MainThreadHandle> thread, Factory factory)
      : AnotherThreadObjectReference(thread, thread->newObjectId()) {
    thread_->Post(NewCreateRequest(object_id_, std::move(factory)));
  }
  AnotherThreadObjectReference(AnotherThreadObjectReference&) = delete;

  ~AnotherThreadObjectReference() {
    // Disappearing thread may cause a memory leak
    thread_->Post(
        std::unique_ptr<DeleteRequest>(new DeleteRequest(object_id_)));
  }

  template <typename Fn>
  void Call(Fn fn) const {
    using Request = CallRequest<T, Fn>;
    thread_->Post(std::unique_ptr<Request>(
        new Request(object_id_, std::move(fn))));
  }

  template <typename Arg>
  void Call(void (T::*fn)(Arg), Arg argument) const {
    Call(std::bind(Apply<Arg>, std::placeholders::_1, fn, std::move(argument)));
  }

 private:
  // This has to use non-const reference to support std::bind with non-copyable
  // types
  template <typename Argument>
  static void Apply(T* target, void (T::*fn)(Argument),
    /* NOLINT (runtime/references) */ Argument& argument) {
    (target->*fn)(std::move(argument));
  }

  std::shared_ptr<MainThreadHandle> thread_;
  const int object_id_;
};

class MainThreadSessionState {
 public:
  MainThreadSessionState(MainThreadInterface* thread, bool prevent_shutdown)
                         : thread_(thread),
                           prevent_shutdown_(prevent_shutdown) {}

  static std::unique_ptr<MainThreadSessionState> Create(
      MainThreadInterface* thread, bool prevent_shutdown) {
    return std::unique_ptr<MainThreadSessionState>(
        new MainThreadSessionState(thread, prevent_shutdown));
  }

  void Connect(std::unique_ptr<InspectorSessionDelegate> delegate) {
    Agent* agent = thread_->inspector_agent();
    if (agent != nullptr)
      session_ = agent->Connect(std::move(delegate), prevent_shutdown_);
  }

  void Dispatch(std::unique_ptr<StringBuffer> message) {
    session_->Dispatch(message->string());
  }

 private:
  MainThreadInterface* thread_;
  bool prevent_shutdown_;
  std::unique_ptr<InspectorSession> session_;
};

class CrossThreadInspectorSession : public InspectorSession {
 public:
  CrossThreadInspectorSession(
      int id,
      std::shared_ptr<MainThreadHandle> thread,
      std::unique_ptr<InspectorSessionDelegate> delegate,
      bool prevent_shutdown)
      : state_(thread, std::bind(MainThreadSessionState::Create,
                                 std::placeholders::_1,
                                 prevent_shutdown)) {
    state_.Call(&MainThreadSessionState::Connect, std::move(delegate));
  }

  void Dispatch(const StringView& message) override {
    state_.Call(&MainThreadSessionState::Dispatch,
                StringBuffer::create(message));
  }

 private:
  AnotherThreadObjectReference<MainThreadSessionState> state_;
};

class ThreadSafeDelegate : public InspectorSessionDelegate {
 public:
  ThreadSafeDelegate(std::shared_ptr<MainThreadHandle> thread, int object_id)
                     : thread_(thread), delegate_(thread, object_id) {}

  void SendMessageToFrontend(const v8_inspector::StringView& message) override {
    delegate_.Call(
        [m = StringBuffer::create(message)]
        (InspectorSessionDelegate* delegate) {
      delegate->SendMessageToFrontend(m->string());
    });
  }

 private:
  std::shared_ptr<MainThreadHandle> thread_;
  AnotherThreadObjectReference<InspectorSessionDelegate> delegate_;
};
}  // namespace


MainThreadInterface::MainThreadInterface(Agent* agent, uv_loop_t* loop,
                                         v8::Isolate* isolate,
                                         v8::Platform* platform)
                                         : agent_(agent), isolate_(isolate),
                                           platform_(platform) {
  main_thread_request_.reset(new AsyncAndInterface(uv_async_t(), this));
  CHECK_EQ(0, uv_async_init(loop, &main_thread_request_->first,
                            DispatchMessagesAsyncCallback));
  // Inspector uv_async_t should not prevent main loop shutdown.
  uv_unref(reinterpret_cast<uv_handle_t*>(&main_thread_request_->first));
}

MainThreadInterface::~MainThreadInterface() {
  if (handle_)
    handle_->Reset();
}

// static
void MainThreadInterface::DispatchMessagesAsyncCallback(uv_async_t* async) {
  AsyncAndInterface* asyncAndInterface =
      node::ContainerOf(&AsyncAndInterface::first, async);
  asyncAndInterface->second->DispatchMessages();
}

// static
void MainThreadInterface::CloseAsync(AsyncAndInterface* pair) {
  uv_close(reinterpret_cast<uv_handle_t*>(&pair->first), DisposePairCallback);
}

void MainThreadInterface::Post(std::unique_ptr<Request> request) {
  Mutex::ScopedLock scoped_lock(requests_lock_);
  bool needs_notify = requests_.empty();
  requests_.push_back(std::move(request));
  if (needs_notify) {
    CHECK_EQ(0, uv_async_send(&main_thread_request_->first));
    if (isolate_ != nullptr && platform_ != nullptr) {
      std::shared_ptr<v8::TaskRunner> taskrunner =
        platform_->GetForegroundTaskRunner(isolate_);
      taskrunner->PostTask(std::make_unique<DispatchMessagesTask>(this));
      isolate_->RequestInterrupt([](v8::Isolate* isolate, void* thread) {
        static_cast<MainThreadInterface*>(thread)->DispatchMessages();
      }, this);
    }
  }
  incoming_message_cond_.Broadcast(scoped_lock);
}

bool MainThreadInterface::WaitForFrontendEvent() {
  // We allow DispatchMessages reentry as we enter the pause. This is important
  // to support debugging the code invoked by an inspector call, such
  // as Runtime.evaluate
  dispatching_messages_ = false;
  if (dispatching_message_queue_.empty()) {
    Mutex::ScopedLock scoped_lock(requests_lock_);
    while (requests_.empty()) incoming_message_cond_.Wait(scoped_lock);
  }
  return true;
}

void MainThreadInterface::DispatchMessages() {
  if (dispatching_messages_)
    return;
  dispatching_messages_ = true;
  bool had_messages = false;
  do {
    if (dispatching_message_queue_.empty()) {
      Mutex::ScopedLock scoped_lock(requests_lock_);
      requests_.swap(dispatching_message_queue_);
    }
    had_messages = !dispatching_message_queue_.empty();
    while (!dispatching_message_queue_.empty()) {
      MessageQueue::value_type task;
      std::swap(dispatching_message_queue_.front(), task);
      dispatching_message_queue_.pop_front();
      task->Call(this);
    }
  } while (had_messages);
  dispatching_messages_ = false;
}

std::shared_ptr<MainThreadHandle> MainThreadInterface::GetHandle() {
  if (handle_ == nullptr)
    handle_ = std::make_shared<MainThreadHandle>(this);
  return handle_;
}

void MainThreadInterface::AddObject(int id,
                                    std::unique_ptr<Deletable> object) {
  CHECK_NE(nullptr, object);
  managed_objects_[id] = std::move(object);
}

void MainThreadInterface::RemoveObject(int id) {
  CHECK_EQ(1, managed_objects_.erase(id));
}

Deletable* MainThreadInterface::GetObject(int id) {
  auto iterator = managed_objects_.find(id);
  // This would mean the object is requested after it was disposed, which is
  // a coding error.
  CHECK_NE(managed_objects_.end(), iterator);
  Deletable* pointer = iterator->second.get();
  CHECK_NE(nullptr, pointer);
  return pointer;
}

std::unique_ptr<StringBuffer> Utf8ToStringView(const std::string& message) {
  icu::UnicodeString utf16 = icu::UnicodeString::fromUTF8(
      icu::StringPiece(message.data(), message.length()));
  StringView view(reinterpret_cast<const uint16_t*>(utf16.getBuffer()),
                  utf16.length());
  return StringBuffer::create(view);
}

std::unique_ptr<InspectorSession> MainThreadHandle::Connect(
    std::unique_ptr<InspectorSessionDelegate> delegate,
    bool prevent_shutdown) {
  return std::unique_ptr<InspectorSession>(
      new CrossThreadInspectorSession(++next_session_id_,
                                      shared_from_this(),
                                      std::move(delegate),
                                      prevent_shutdown));
}

bool MainThreadHandle::Post(std::unique_ptr<Request> request) {
  Mutex::ScopedLock scoped_lock(block_lock_);
  if (!main_thread_)
    return false;
  main_thread_->Post(std::move(request));
  return true;
}

void MainThreadHandle::Reset() {
  Mutex::ScopedLock scoped_lock(block_lock_);
  main_thread_ = nullptr;
}

std::unique_ptr<InspectorSessionDelegate>
MainThreadHandle::MakeDelegateThreadSafe(
    std::unique_ptr<InspectorSessionDelegate> delegate) {
  int id = newObjectId();
  main_thread_->AddObject(id, WrapInDeletable(std::move(delegate)));
  return std::unique_ptr<InspectorSessionDelegate>(
      new ThreadSafeDelegate(shared_from_this(), id));
}

bool MainThreadHandle::Expired() {
  Mutex::ScopedLock scoped_lock(block_lock_);
  return main_thread_ == nullptr;
}
}  // namespace inspector
}  // namespace node
