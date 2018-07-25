'use strict';

const ChildProcess = require('child_process');

let last_unique_id = 0;

const generateUniqueId = function () {
    last_unique_id++;
    return '_' + last_unique_id;
};

/**
 * @returns {{
 *   setMaxProcesses: (function(int)),
 *   getMaxProcesses: (function(): int),
 *   getCurrentProcessCount: (function(): int),
 *   getCurrentProcesses: (function(): Array.<ChildProcess>),
 *   getCurrentQueueSize: (function(): int),
 *   removeFromQueue: (function(String): Boolean),
 *   fork: (function(...[*]): String),
 *   spawn: (function(...[*]): String),
 *   exec: (function(...[*]): String),
 *   exec1: (function(...[*]): String),
 *   execFile: (function(...[*]): String),
 *   newQueue: newQueue
 * }}
 */
const newQueue = function newQueue() {

    let MAX_PROCESSES = 5;

    let QUEUE = [];
    let MAP = new Map();
    let PROCESSES = [];

    let tryToReleaseQueue = function () {

        if (PROCESSES.length >= MAX_PROCESSES || !QUEUE.length) {
            return false;
        }

        let next = QUEUE.shift();
        MAP.delete(next.id);

        let args = next.args;
        let process;

        if (next.hasTerminateCallback) {
            let oldTerminateCallback;
            if (typeof args[args.length - 1] === 'function') {
                oldTerminateCallback = args[args.length - 1];
            }

            //noinspection UnnecessaryLocalVariableJS
            let terminateCallback = function () {

                removeTerminatedProcess(process);

                if (oldTerminateCallback) {
                    oldTerminateCallback.apply(this, arguments);
                }
            };

            args[oldTerminateCallback ? args.length - 1 : args.length] = terminateCallback;
        }

        process = ChildProcess[next.func].apply(ChildProcess, args);
        PROCESSES.push(process);

        process.on('exit', function () {
            removeTerminatedProcess(process);
        });

        if (next.callback) {
            next.callback(process);
        }

        return true;
    };

    let removeTerminatedProcess = function (process) {

        for (let i = 0; i < PROCESSES.length; i++) {
            if (PROCESSES[i] === process) {
                PROCESSES.splice(i, 1);
                break;
            }
        }

        setImmediate(tryToReleaseQueue);
    };

    let cloneObject = function (o) {
        let clone = {};

        if (o === null || typeof o !== 'object') {
            return clone;
        }

        let keys = Object.keys(o);
        for (let i = keys.length - 1; i >= 0; i--) {
            clone[keys[i]] = o[keys[i]];
        }

        return clone;
    };

    let extractOnCreateFromArgs = function (args) {

        let i = 0;
        let length = args.length;
        for (; i < length; i++) {
            if (Array.isArray(args[i])) {
                continue;
            }
            if (typeof args[i] !== 'object') {
                continue;
            }

            let options = cloneObject(args[i]);
            let onCreate = options['onCreate'];
            delete options['onCreate'];
            args[i] = options;
            return onCreate;
        }

        return null;
    };

    //noinspection JSUnusedGlobalSymbols
    let improvedChildProcess = {

        setMaxProcesses: function setMaxProcesses(max) {
            MAX_PROCESSES = max || 5;
            tryToReleaseQueue();
            return this;
        },

        getMaxProcesses: function getMaxProcesses() {
            return MAX_PROCESSES;
        },

        getCurrentProcessCount: function getCurrentProcessCount() {
            return PROCESSES.length;
        },

        getCurrentProcesses: function getCurrentProcesses() {
            return PROCESSES.slice(0);
        },

        getCurrentQueueSize: function getCurrentQueueSize() {
            return QUEUE.length;
        },

        removeFromQueue: function (id) {

            if (MAP.has(id)) {
                let item = MAP.get(id);
                MAP.delete(id);

                let index = QUEUE.indexOf(item);
                if (index !== -1)
                    QUEUE.splice(index, 1);

                return true;
            }

            return false;
        },

        fork: function fork(modulePath /*, args, options*/) {

            let args = Array.prototype.slice.call(arguments, 0);
            let onCreate = extractOnCreateFromArgs(args);

            let task = {
                func: 'fork',
                args: args,
                callback: onCreate,
                hasTerminateCallback: false,
                id: generateUniqueId()
            };
            QUEUE.push(task);
            MAP.set(task.id, task);

            tryToReleaseQueue();

            return task.id;
        },

        spawn: function spawn(command /*, args, options*/) {

            let args = Array.prototype.slice.call(arguments, 0);
            let onCreate = extractOnCreateFromArgs(args);

            let task = {
                func: 'spawn',
                args: args,
                callback: onCreate,
                hasTerminateCallback: false,
                id: generateUniqueId()
            };
            QUEUE.push(task);
            MAP.set(task.id, task);

            tryToReleaseQueue();

            return task.id;
        },

        exec: function exec(command /*, options, callback*/) {

            let args = Array.prototype.slice.call(arguments, 0);
            let onCreate = extractOnCreateFromArgs(args);

            let task = {
                func: 'exec',
                args: args,
                callback: onCreate,
                hasTerminateCallback: true,
                id: generateUniqueId()
            };
            QUEUE.push(task);
            MAP.set(task.id, task);

            tryToReleaseQueue();

            return task.id;
        },

        execFile: function execFile(file /*, args, options, callback*/) {

            let args = Array.prototype.slice.call(arguments, 0);
            let onCreate = extractOnCreateFromArgs(args);

            let task = {
                func: 'exec',
                args: args,
                callback: onCreate,
                hasTerminateCallback: true,
                id: generateUniqueId()
            };
            QUEUE.push(task);
            MAP.set(task.id, task);

            tryToReleaseQueue();

            return task.id;
        }

    };

    improvedChildProcess.newQueue = newQueue;

    //noinspection JSValidateTypes
    return improvedChildProcess;

};

/**
 * @type {{
 *   setMaxProcesses: (function(int)),
 *   getMaxProcesses: (function(): int),
 *   getCurrentProcessCount: (function(): int),
 *   getCurrentProcesses: (function(): Array.<ChildProcess>),
 *   getCurrentQueueSize: (function(): int),
 *   removeFromQueue: (function(String): Boolean),
 *   fork: (function(...[*]): String),
 *   spawn: (function(...[*]): String),
 *   exec: (function(...[*]): String),
 *   execFile: (function(...[*]): String),
 *   newQueue: newQueue
 * }}
 */
module.exports = newQueue();