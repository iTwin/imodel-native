/*---------------------------------------------------------------------------------------------
* Copyright (c) Bentley Systems, Incorporated. All rights reserved.
* See LICENSE.md in the project root for license terms and full copyright notice.
*--------------------------------------------------------------------------------------------*/

import { expect } from "chai";
import * as sinon from "sinon";
import { BeDuration, Logger, LogLevel, StopWatch, using } from "@itwin/core-bentley";
import { iModelJsNative } from "./utils";

describe("Logger", () => {
  const testCategory = "test-category";

  before(async () => {
    Logger.setLevel(testCategory, LogLevel.Trace);
    iModelJsNative.clearLogLevelCache();
  });

  after(() => {
    Logger.setLevel(testCategory, LogLevel.None);
  });

  afterEach(() => {
    sinon.restore();
  });

  it("logs messages from main thread", async () => {
    const logTrace = sinon.stub(iModelJsNative.logger, "logTrace");
    await new Promise<void>((resolve) => {
      iModelJsNative.NativeDevTools.emitLogs(3, testCategory, LogLevel.Trace, "main", resolve);
    });
    await waitFor(() => expect(logTrace.callCount).to.eq(3));
  });

  it("handles errors thrown in JS when logging on the main thread", async () => {
    const logTrace = sinon.stub(iModelJsNative.logger, "logTrace").throws(new Error("test error"));
    await expect(new Promise<void>((resolve, reject) => {
      try {
        iModelJsNative.NativeDevTools.emitLogs(2, testCategory, LogLevel.Trace, "main", resolve);
      } catch (e) {
        reject(e instanceof Error ? e : new Error(String(e)));
      }
    })).to.eventually.be.rejectedWith("test error");
    expect(logTrace.calledOnce).to.be.true;
  });

  it("logs messages from worker thread", async () => {
    const logTrace = sinon.stub(iModelJsNative.logger, "logTrace");
    await new Promise<void>((resolve) => {
      iModelJsNative.NativeDevTools.emitLogs(3, testCategory, LogLevel.Trace, "worker", resolve);
    });
    await waitFor(() => expect(logTrace.callCount).to.eq(3));
  });

  it("silently ignores errors thrown in JS when logging on the worker thread", async () => {
    const logTrace = sinon.stub(iModelJsNative.logger, "logTrace").throws(new Error("test error"));
    await new Promise<void>((resolve, reject) => {
      try {
        iModelJsNative.NativeDevTools.emitLogs(2, testCategory, LogLevel.Trace, "worker", resolve);
      } catch (e) {
        reject(e instanceof Error ? e : new Error(String(e)));
      }
    });
    expect(logTrace.calledTwice).to.be.true;
  });

  it("doesn't block main thread while emitting worker thread logs", async () => {
    /**
     * Monitors whether the event loop is blocked or not by registering an interval without a timeout
     * and counting how many times it was triggered.
     */
    class MainThreadMonitor {
      private _timer: NodeJS.Timeout | undefined;
      private _triggerCount = 0;

      public start() {
        this._timer = setInterval(() => this.onInterval(), 1);
      }
      public dispose() {
        if (this._timer) {
          clearInterval(this._timer);
        }
      }
      private onInterval() {
        ++this._triggerCount;
      }
      public get triggerCount() {return this._triggerCount;}
    }

    const count = 50;
    let onFirstEmission: () => void;
    let isFirstEmission = true;
    const logTrace = sinon.stub(iModelJsNative.logger, "logTrace").withArgs(testCategory, sinon.match.string);
    logTrace.callsFake(() => {
      if (isFirstEmission) {
        onFirstEmission();
        isFirstEmission = false;
      }
    });

    await using(new MainThreadMonitor(), async (monitor) => {
      onFirstEmission = () => monitor.start();
      await new Promise<void>((resolve) => {
        iModelJsNative.NativeDevTools.emitLogs(count, testCategory, LogLevel.Trace, "worker", resolve);
      });
      await waitFor(() => expect(logTrace.callCount).to.eq(count));
      expect(monitor.triggerCount).to.be.above(0);
    });
  });
});

/**
 * Waits until `check` doesn't throw or a timeout happens. In case the `check` succeeds before the timeout,
 * it's result is returned. In case of a timeout, the last error, thrown by calling `check`, is re-thrown.
 */
async function waitFor<T>(check: () => Promise<T> | T, timeout: number = 5000): Promise<T> {
  const timer = new StopWatch(undefined, true);
  let lastError: unknown;
  do {
    try {
      const res = check();
      return res instanceof Promise ? await res : res;
    } catch (e) {
      lastError = e;
      await BeDuration.wait(0);
    }
  } while (timer.current.milliseconds < timeout);
  throw lastError;
}
