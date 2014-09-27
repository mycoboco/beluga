#!/usr/bin/node

var assert = require('assert');
var EventEmitter = require('events').EventEmitter,
    ee = new EventEmitter();
var fs = require('fs');
var path = require('path');
var spawn = require('child_process').spawn;


var prgname = 'run.js';
var dir;
var elist = {};
var flist = [];
var run = {
    'beluga\'s diagnostics': {
        exec: '../../build/beluga',
        copt: [ '--errstop=0', '--hexcode' ],
        eopt: [ '-Wv', '--std=c90' ],
        tout: [ null, false, true ]              // stderr only
    },
    'sea-canary': {
        exec: '../../build/sc',
        copt: [ '--errstop=0', '--hexcode' ],
        eopt: [ '-Wv', '--std=c90' ],
        tout: [ null, true, true ]
    },
    'mcpp\'s testcases': {
        exec: '../../build/sc',
        copt: [ '--errstop=0', '--hexcode' ],
        eopt: [ '-Wv', '--std=c90' ],
        tout: [ null, true, true ]
    }
};


var prep = function () {
    var idx = (path.basename(process.argv[0]).indexOf('node') >= 0)? 1: 0;

    var err = function (msg) {
        console.log(prgname + ': ' + msg);
        process.exit(1);
    };

    if (!process.argv[idx])
        prgname = path.basename(process.argv[idx]);

    if (!process.argv[++idx])
        err('no working directory given');
    dir = process.argv[idx];

    try {
        id = fs.readFileSync(path.join(dir, 'ID'), 'utf8');
    } catch(e) {
        err('no test identification set');
    }
    id = id.replace(/\n/g, '');
};


var exclude = function () {
    var buf;

    try {
        buf = fs.readFileSync(path.join(dir, 'EXCLUDE'), 'utf8');
    } catch(e) {
        return;
    }

    buf = buf.split('\n');
    console.log('\n- following files will not be used:');
    for (var i = 0; i < buf.length; i++)
        if (buf[i]) {
            elist[buf[i]] = true;
            console.log('  ' + buf[i]);
        }
};


var proc = function (name) {
    var opt;
    var code;

    var next = function (fail, msg) {
        if (fail) {
            process.stdout.write('[FAILED]' + ((msg)? ': '+msg: '') + '\n');
            flist.push(name);
        } else
            process.stdout.write('[ok]\n');
        ee.emit('next');
    };

    var bufeq = function (b1, b2) {
        if (b1.length !== b2.length)
            return false;
        for (var i = 0; i < b1.length; i++)
            if (b1[i] !== b2[i])
                return false;
        return true;
    };

    process.stdout.write('  checking for ' + name + '... ');

    try {
        code = fs.readFileSync(path.join(dir, name), 'binary');
    } catch(e) {
        next(true, 'cannot read test code');
        return;
    }

    code = code.split('\n');
    if (code[0].substring(0, 2) === '/*' &&
        (opt=/\/\*(.*) \*\//.exec(code[0]), opt && opt[1])) {
        opt = opt[1].split(' -');
        for (var i = 0; i < opt.length; ) {
            if (!opt[i]) {
                opt.splice(i, 1);
                continue;
            } else
//                opt[i] = ('-' + opt[i]).replace(/"/g, '');
                opt[i] = ('-' + opt[i])
            i++;
        }
    } else
        opt = run[id].eopt;
    opt = opt.concat(run[id].copt);
    opt.push('./' + name);

    (function () {
        var stdout = new Buffer(0),
            stderr = new Buffer(0);

        child = spawn(run[id].exec, opt, { cwd: dir });
        child.stdout.on('data', function (data) {
            stdout = Buffer.concat([ stdout, data ]);
        });
        child.stderr.on('data', function (data) {
            stderr = Buffer.concat([ stderr, data ]);
        });

        child.on('close', function () {
            var fail, msg;
            var source = [ null, stdout, stderr ];
            var origin = [];

            for (var i = 1; i <= 2; i++) {
                if (!run[id].tout[i])
                    continue;
                try {
                    origin[i] = fs.readFileSync(path.join(dir, name+'.'+i+'.out'));
                } catch(e) {
                    origin[i] = undefined;
                }
                if (!origin[i] || !bufeq(source[i], origin[i])) {
                    fail = true;
                    try {
                        fs.writeFileSync(path.join(dir, name+'.'+i+'.out.new'), source[i]);
                    } catch(e) {
                        msg = 'cannot write output file';
                    }
                }
            }

            next(fail, msg);
        });
    })();
};


// starts here
(function () {
    var buf;
    var idx = 0;
    var list = [];

    prep();
    console.log('Running tests for ' + id + ':');

    exclude();
    buf = fs.readdirSync(dir);
    for (var i = 0; i < buf.length; i++)
        if (/[a-z0-9\-].c$/i.test(buf[i]) && !elist[buf[i]])
            list.push(buf[i]);

    console.log('');
    ee.on('next', function () {
        if (idx < list.length)
            proc(list[idx++]);
        else if (flist.length > 0) {
            console.log('\n- test failed for the following files:');
            for (var i = 0; i < flist.length; i++)
                console.log('  ' + flist[i]);
        } else
            console.log('\n- all ' + list.length + ' tests passed');
    }).emit('next');
})();

// end of run.js
