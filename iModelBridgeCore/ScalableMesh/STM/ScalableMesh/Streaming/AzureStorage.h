#pragma once

// https://azure.microsoft.com/en-us/documentation/articles/storage-c-plus-plus-how-to-use-blobs/

#include <wastorage/was/storage_account.h>
#include <wastorage/was/blob.h>
#include <wastorage/was/table.h>

#include <cpprest/filestream.h>

#ifndef NDEBUG
#define DEBUG_AZURE
#endif

#ifdef DEBUG_AZURE
#include <Bentley\BeConsole.h>
#endif

namespace scalable_mesh
    {
    using namespace azure;
    namespace azure {
        class Storage;

        typedef storage::cloud_storage_account account;
        typedef storage::cloud_blob_client     blobclient;
        typedef storage::cloud_table_client    tableclient;
        typedef storage::cloud_blob_container  container;
        typedef storage::cloud_table           table;
        typedef storage::cloud_blob            azure_blob;
        typedef storage::cloud_block_blob      azure_block_blob;
        typedef std::shared_ptr<Storage>       StoragePtr;

        class Storage {

        public:
            typedef std::vector<uint8_t> point_buffer_type;
            typedef concurrency::streams::container_buffer<std::vector<uint8_t>> point_buffer_type2;

        private:
            account m_storage;
            blobclient m_bclient;
            tableclient m_tclient;

            container m_container;

        public:
            Storage() {
                // Define the connection-string with Azure Storage Emulator (Default behavior).
                m_storage = storage::cloud_storage_account::parse(U("UseDevelopmentStorage=true;"));

                // Create the blob client.
                m_bclient = m_storage.create_cloud_blob_client();

                // Create the table client.
                m_tclient = m_storage.create_cloud_table_client();

                }

            Storage(const std::wstring& connection_string)
                {
                // Retrieve storage account from connection string.
                m_storage = storage::cloud_storage_account::parse(connection_string);

                // Create the blob client.
                m_bclient = m_storage.create_cloud_blob_client();

                // Create the table client.
                m_tclient = m_storage.create_cloud_table_client();
                }

            Storage(const std::wstring& connection_string, const std::wstring& container_name)
                {
                try {
                    // Retrieve storage account from connection string.
                    m_storage = storage::cloud_storage_account::parse(connection_string);

                    // Create the blob client.
                    m_bclient = m_storage.create_cloud_blob_client();

                    // Create the table client.
                    m_tclient = m_storage.create_cloud_table_client();

                    m_container = GetContainer(container_name).get();
                    }
                catch (...) {
                    assert(!"Cloud problem...");
                    }
                }

            pplx::task<container> GetContainer(const std::wstring& name)
                {
                // Retrieve a reference to a container.
                container container = m_bclient.get_container_reference(name);

                return container.create_if_not_exists_async().then([container, name](const bool& created)
                    {
                    if (!created && container.list_blobs() == storage::list_blob_item_iterator())
                        {   // container not created and an existing container is not empty
                        throw std::wstring(L"Warning: Point cloud \"" + name + L"\" already exists and will not be overwritten (point clouds are immutable)");
                        }
                    return container;

                    });
                }

            azure_block_blob GetBlockBlob(const std::wstring& name) const
                {
                return m_container.get_block_blob_reference(name);
                }

            azure_block_blob GetBlockBlob(const container& container, const std::wstring& name) const
                {
                return container.get_block_blob_reference(name);
                }

            pplx::task<bool> DeletePointCloud(const std::wstring& name)
                {
                // Retrieve a reference to a container.
                container container = m_bclient.get_container_reference(name);
                return container.delete_container_if_exists_async();
                }

            void CreateBlob(const std::wstring& blobName, const std::wstring& containerName)
                {

                // Retrieve a reference to a container.
                container container = m_bclient.get_container_reference(containerName);

                // Create the container if it doesn't already exist.
                container.create_if_not_exists();

                // Retrieve reference to a blob named "my-blob-1".
                storage::cloud_block_blob blockBlob = container.get_block_blob_reference(U("my-blob-1"));

                // Create or overwrite the "my-blob-1" blob with contents from a local file.
                concurrency::streams::istream input_stream = concurrency::streams::file_stream<uint8_t>::open_istream(U("DataFile.txt")).get();
                blockBlob.upload_from_stream(input_stream);
                input_stream.close().wait();

                // Create or overwrite the "my-blob-2" and "my-blob-3" blobs with contents from text.
                // Retrieve a reference to a blob named "my-blob-2".
                storage::cloud_block_blob blob2 = container.get_block_blob_reference(U("my-blob-2"));
                blob2.upload_text(U("more text"));

                // Retrieve a reference to a blob named "my-blob-3".
                storage::cloud_block_blob blob3 = container.get_block_blob_reference(U("my-directory/my-sub-directory/my-blob-3"));
                blob3.upload_text(U("other text"));
                }

            storage::container_result_iterator list_containers()
                {
                return m_bclient.list_containers();
                }

            void ListAllContainers()
                {
                storage::container_result_iterator end_container_iterator;
                for (auto it = m_bclient.list_containers(); it != end_container_iterator; it++)
                    {
                    std::wcout << U("Container: ") << it->name() << std::endl;
                    }
                }

            concurrency::task<std::string> GetContainerInfo(container container)
                {
                auto num_blobs = std::make_shared<int>(0);
                auto num_dirs = std::make_shared<int>(0);
                auto size = std::make_shared<uint64_t>(0);

                std::function<void(storage::list_blob_item_segment)> accumulateBlobTypes = [&container, &accumulateBlobTypes, num_blobs, num_dirs, size](storage::list_blob_item_segment blob_segment)
                    {
                    for (auto blob : blob_segment.results())
                        {
                        /*if (blob.is_blob())
                        {
                        std::wcout << U("    Blob: ") << blob.as_blob().uri().primary_uri().to_string() << std::endl;
                        }
                        else
                        {
                        std::wcout << U("    Directory: ") << blob.as_directory().uri().primary_uri().to_string() << std::endl;
                        }*/
                        if (blob.is_blob())
                            {
                            *size += blob.as_blob().properties().size();
                            ++(*num_blobs);
                            }
                        else ++(*num_dirs);
                        }
                    if (!blob_segment.continuation_token().empty()) container.list_blobs_segmented_async(storage::continuation_token()).then(accumulateBlobTypes);

                    };
                return container.list_blobs_segmented_async(storage::continuation_token()).then(accumulateBlobTypes).then([container, num_blobs, num_dirs, size]()
                    {
                    std::string result;
                    std::wcout << U("Container: ") << container.name() << std::endl
                        << U("    ETag: ") << container.properties().etag() << std::endl
                        << U("    Size: ") << (*size >> 20) << " MB" << std::endl
                        << U("    #Blobs: ") << *num_blobs << std::endl
                        << U("    #Directories: ") << *num_dirs << std::endl;
                    return result;

                    });
                }

            pplx::task<void> GetPoints(const std::wstring& point_cloud, std::function<void(point_buffer_type)> callback)
                {
                // Retrieve a reference to the container.
                container container = m_bclient.get_container_reference(point_cloud);
                if (!container.is_valid() || !container.exists()) return pplx::create_task([]() {});

                return this->DownloadAllBlobsAsync(container, callback);
                }

            pplx::task<void> GetPointsFromAll(std::function<void(point_buffer_type)> callback)
                {
                std::vector<pplx::task<void>> tasks;
                storage::container_result_iterator end_container_iterator;
                for (auto it = this->list_containers(); it != end_container_iterator; it++)
                    {
                    tasks.push_back(this->DownloadAllBlobsAsync(*it, callback));
                    }
                return pplx::when_all(tasks.begin(), tasks.end());
                }

            // Slow synchronous download
            pplx::task<void> DownloadAllBlobs(container container)
                {
                std::function<void(storage::list_blob_item_segment)> accumulateBlobTypes = [&container, &accumulateBlobTypes](storage::list_blob_item_segment blob_segment)
                    {
                    for (auto blob : blob_segment.results())
                        {
                        if (blob.is_blob())
                            {
                            concurrency::streams::container_buffer<std::vector<uint8_t>> buffer;
                            concurrency::streams::ostream stream(buffer);
                            blob.as_blob().download_to_stream(stream);
                            std::wcout << U(".");
                            }
                        }
                    if (!blob_segment.continuation_token().empty()) container.list_blobs_segmented_async(storage::continuation_token()).then(accumulateBlobTypes);

                    };
                return container.list_blobs_segmented_async(storage::continuation_token()).then(accumulateBlobTypes).then([container]()
                    {
                    std::wcout << U("Done downloading blobs from ") << container.name() << std::endl;
                    });
                }

            // Fast parallel download
            pplx::task<void> DownloadAllBlobsAsync(container container, std::function<void(point_buffer_type)> callback)
                {
                // Blob downloads are segmented into chunks of blobs (up to 5000 blobs by default).
                // Using a token allows for asynchronous downloads of those chunks. Here we 
                // simply create a task for each segments.
                std::vector<pplx::task<void>> tasks;
                storage::continuation_token token;
                do {
                    tasks.push_back(container.list_blobs_segmented_async(token).then([this, callback](storage::list_blob_item_segment blob_segment)
                        {
                        return this->DownloadBlobSegment(blob_segment, callback);
                        }));
                    } while (!token.empty());

                    return pplx::when_all(tasks.begin(), tasks.end()).then([container]()
                        {
                        std::wcout << U("Done downloading segments from ") << container.name() << std::endl;
                        });
                }

            void DownloadBlob(const std::wstring& blob_name, std::function<void(point_buffer_type)> callback) const
                {
#ifdef DEBUG_AZURE
                static std::atomic<int> s_parallelCalls = 0;
                static std::mutex s_consoleMutex;

                s_parallelCalls += 1;

                s_consoleMutex.lock();
                wprintf(L"Threads in DownloadBlob : %i      blob name: %s \r\n", (int)s_parallelCalls, blob_name.c_str());
                //BeConsole::Printf("Threads in DownloadBlob : %i      blob name: %s \r\n", s_parallelCalls);
                //BeConsole::WPrintf(L"blob name : %s \r\n", blob_name.c_str());
                s_consoleMutex.unlock();
#endif
                point_buffer_type2 buffer;
                try
                    {
                    auto block_blob = this->GetBlockBlob(blob_name);
                    assert(block_blob.is_valid() && block_blob.exists());
                    block_blob.download_to_stream(concurrency::streams::ostream(buffer));
                    callback(buffer.collection());
                    }
                catch (const std::exception& e)
                    {
                    std::wcout << U("Error: ") << e.what() << std::endl;
                    assert(!"There is an error downloading from Azure");
                    }
#ifdef DEBUG_AZURE
                s_parallelCalls -= 1;
#endif
                }

            void DownloadBlobRange(const std::wstring& blob_name, const uint64_t& offset, const uint64_t& length, std::function<void(point_buffer_type)> callback) const
                {
                point_buffer_type2 buffer;
                auto block_blob = this->GetBlockBlob(blob_name);
                if (block_blob.is_valid() && block_blob.exists())
                    {
                    try
                        {
                        block_blob.download_range_to_stream(concurrency::streams::ostream(buffer), offset, length);
                        callback(buffer.collection());
                        }
                    catch (const std::exception& e)
                        {
                        std::wcout << U("Error: ") << e.what() << std::endl;
                        }
                    }
                }

            pplx::task<void> DownloadBlob(storage::list_blob_item blob_item, std::function<void(point_buffer_type)> callback) const
                {
                if (blob_item.is_blob())
                    {
                    //std::wcout << U("b");
                    point_buffer_type2 buffer;
                    return blob_item.as_blob().download_to_stream_async(concurrency::streams::ostream(buffer)).then([buffer, callback]()
                        {
                        callback(buffer.collection());
                        });
                    }
                return pplx::task<void>();
                }

            pplx::task<void> DownloadBlobSegment(storage::list_blob_item_segment blob_segment, std::function<void(point_buffer_type)> callback)
                {
                std::wcout << U("Downloading a segment containing ") << blob_segment.results().size() << " blobs...";
                return pplx::create_task([this, blob_segment, callback]()
                    {
                    std::vector<pplx::task<void>> blobs;
                    for (auto blob_item : blob_segment.results())
                        {
                        blobs.push_back(this->DownloadBlob(blob_item, callback));
                        }
                    pplx::when_all(blobs.begin(), blobs.end()).then([]()
                        {
                        std::wcout << U("Done with segment...") << std::endl;
                        }).get(); // force task to wait for all blobs to download within the current segment
                    });
                }

            /*pplx::task<void> Print(std::function<void(std::string)> callback)
            {
            std::string result;
            std::vector<pplx::task<void>> tasks;
            storage::container_result_iterator end_container_iterator;
            for (auto it = m_bclient.list_containers(); it != end_container_iterator; it++)
            {
            tasks.push_back(this->ListAllBlobsFrom(*it));
            }
            auto invoke_callback = [callback, result]() {
            try {
            callback(result);
            }
            catch (const std::exception& e)
            {
            std::wcout << U("Error: ") << e.what() << std::endl;
            }
            };
            return pplx::when_all(tasks.begin(), tasks.end()).then(invoke_callback);
            }*/

            /*pplx::task<std::string> GetInfo()
                {
                std::string result;
                storage::container_result_iterator end_container_iterator;
                for (auto it = m_bclient.list_containers(); it != end_container_iterator; it++)
                    {
                    result = __await this->GetContainerInfo(*it);
                    }
                result = "Done printing...";
                return result;
                }*/

            void CreateTable(const std::wstring& tableName)
                {
                /*boost::uuids::string_generator str_gen;
                boost::uuids::uuid generatedUUID = str_gen(scene_info.name);
                std::cout << boost::uuids::to_string(generatedUUID) << std::endl;*/
                return;

                //auto uuid = utility::string_to_uuid(utility::conversions::to_string_t(scene_info.name));
                auto uuid = utility::new_uuid();


                // Retrieve a reference to a table.
                table table = m_tclient.get_table_reference(L"pointclouds");

                // Create the table if it doesn't exist.
                table.create_if_not_exists();

                // Create point cloud node entity
                storage::table_entity point_cloud_info(utility::uuid_to_string(uuid), L"threemx");

                storage::table_entity::properties_type& properties = point_cloud_info.properties();
                properties.reserve(2);

                //properties[U("name")] = storage::entity_property(utility::conversions::to_string_t(scene_info.name));
                //properties[U("description")] = storage::entity_property(utility::conversions::to_string_t(scene_info.description));


                /*properties[U("container-id")] = storage::entity_property(tableName);
                properties[U("blob-id")] = storage::entity_property(U("000006bis"));
                properties[U("children")] = storage::entity_property(U(""));*/

                // Create the table operation that inserts the customer entity.
                storage::table_operation insert_operation = storage::table_operation::insert_entity(point_cloud_info);

                // Execute the insert operation.
                storage::table_result insert_result = table.execute(insert_operation);
                }

            void AddEntriesToTable(const std::wstring& tableName)
                {
                // Retrieve a reference to a table.
                table table = m_tclient.get_table_reference(tableName);

                // Define a batch operation.
                storage::table_batch_operation batch_operation;

                // Create a customer entity and add it to the table.
                storage::table_entity customer1(U("Smith"), U("Jeff"));

                storage::table_entity::properties_type& properties1 = customer1.properties();
                properties1.reserve(2);
                properties1[U("Email")] = storage::entity_property(U("Jeff@contoso.com"));
                properties1[U("Phone")] = storage::entity_property(U("425-555-0104"));

                // Create another customer entity and add it to the table.
                storage::table_entity customer2(U("Smith"), U("Ben"));

                storage::table_entity::properties_type& properties2 = customer2.properties();
                properties2.reserve(2);
                properties2[U("Email")] = storage::entity_property(U("Ben@contoso.com"));
                properties2[U("Phone")] = storage::entity_property(U("425-555-0102"));

                // Create a third customer entity to add to the table.
                storage::table_entity customer3(U("Smith"), U("Denise"));

                storage::table_entity::properties_type& properties3 = customer3.properties();
                properties3.reserve(2);
                properties3[U("Email")] = storage::entity_property(U("Denise@contoso.com"));
                properties3[U("Phone")] = storage::entity_property(U("425-555-0103"));

                // Add customer entities to the batch insert operation.
                batch_operation.insert_or_replace_entity(customer1);
                batch_operation.insert_or_replace_entity(customer2);
                batch_operation.insert_or_replace_entity(customer3);

                // Execute the batch operation.
                std::vector<storage::table_result> results = table.execute_batch(batch_operation);

                }

            void RemoveAllEmptyContainers()
                {
                storage::container_result_iterator end_container_iterator;
                for (auto it = m_bclient.list_containers(); it != end_container_iterator; it++)
                    {
                    auto blob_segment = it->list_blobs_segmented(storage::continuation_token());
                    if (blob_segment.continuation_token().empty() && blob_segment.results().empty())
                        {
                        container(*it).delete_container_if_exists_async();
                        }
                    }
                }
            };
        }
    }