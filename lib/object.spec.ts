import {IObj, Obj} from './object'
import {Document} from './document'
import * as test from 'tape'
import {join} from 'path';
import {existsSync, unlinkSync} from 'fs'
import {Dictionary} from './dictionary';
import {Arr} from "./arr";

const filePath = join(__dirname, '../test.pdf'),
    outFile = './test.out.pdf',
    outTxtFile = './test.out.txt'

const doc = new Document(filePath),
    obj = doc.getObjects()[0]

test('obj write sync', t => {

    obj.writeSync(outTxtFile)
    t.ok(existsSync(outTxtFile), 'write to destination successful')
    unlinkSync(outTxtFile)
    t.end()
})
test('obj write async', t => {
    try {
        t.plan(3)
        obj.write(outTxtFile, (e, v) => {
            t.assert(e === null, 'error is null')
            t.assert(v === outTxtFile, 'returns destination file path')
            t.ok(existsSync(outTxtFile), 'write to destination successful')
            unlinkSync(outTxtFile)
            t.end()
        })
    }
    catch (e) {
        console.log(e)
        t.fail()
    }

})
test('obj getOffset', t => {
    t.plan(2)
    const dict = new Dictionary(obj),
        keys = dict.getKeys(),
        sync = obj.getOffsetSync(keys[0])
    obj.getOffset(keys[0], (e, v) => {
        t.assert(e === null, 'error is null')
        t.assert(v === sync, 'sync and async return same value')
        t.end()
    })
})

test('obj get as T', t => {
    try {
        const obj = doc.getObjects().filter(i => i.type === 'Array')[0],
            sync = obj.as(obj.type)
        t.ok(sync instanceof Arr, 'is instance of Arr')
        t.end()
    }
    catch (e) {
        console.log(e)
        t.fail('threw error')
    }
})

test('dictionary construct', t => {
    const obj = doc.getObjects().filter(i => i.type === 'Dictionary')[0],
        dict = new Dictionary(obj)
    t.ok(dict, 'Dictionary not null')
    t.assert(dict instanceof Dictionary, 'is instance of Dictionary')
    t.end()
})
test('dictionary writer', t => {
    const obj = doc.getObjects().filter(i => i.type === 'Dictionary')[0],
        dict = new Dictionary(obj),
        out = './test.out.txt'
    dict.writeSync(out)
    t.assert(existsSync(out))
    unlinkSync(out)
    dict.write(out, (e, v) => {
        t.assert(e === null, 'error is null')
        t.ok(existsSync(out), 'write file exists')
        unlinkSync(out)
        t.end()
    })

})
test('array constructor', t => {
    try {
        const obj = doc.getObjects().filter(i => i.type === 'Array')[0],
            arr = obj.as('Array')
        t.assert(arr instanceof Arr, 'instance of Arr')
        t.end()
    }
    catch (e) {
        console.log(e)
        t.fail('threw error')
    }
})
test('Arr to js Array', t => {
    try {
        const obj = doc.getObjects().filter(i => i.type === 'Array')[0],
            arr = obj.as('Array'),
            js = arr.toArray()
        t.assert(js instanceof Array)
        t.end()
    }
    catch (e) {
        console.log(e)
        t.fail('threw error')
    }
})

test('to js', t => {
    try {
        const obj = doc.getObjects().filter(i => i.type === 'Dictionary')[0],
            o = obj.as('Dictionary'),
            js = o.toObject()
        t.assert(js instanceof Object)
        t.end()
    }
    catch (e) {
        console.log(e)
        t.fail('threw error')
    }
})