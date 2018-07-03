/**
 * This file is part of the NoPoDoFo (R) project.
 * Copyright (c) 2017-2018
 * Authors: Cory Mickelson, et al.
 *
 * NoPoDoFo is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NoPoDoFo is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
import { Stream } from "stream";
import { IDictionary } from "./object";
import { IObj } from "./object"
import { IDocument } from "./document"

export type NPDFInternal = any

export type NPDFCoerceKeyType = 'boolean' | 'long' | 'name' | 'real'

export type NPDFDataType = 'Boolean' | 'Number' | 'Name' | 'Real' | 'String' | 'Array' |
    'Dictionary' | 'Reference' | 'RawData'

export interface IObj {
    readonly reference: {object:number, generation:number}
    readonly length: number
    readonly stream: Stream
    readonly type: NPDFDataType
    immutable: boolean

    hasStream(): boolean

    getOffset(key: string): Promise<number>

    write(output: string, cb: Function): void

    flateCompressStream(): void

    delayedStreamLoad(): void

    getBool(): boolean

    getDictionary(): IDictionary

    getString(): string

    getName(): string

    getReal(): number

    getNumber(): number

    getArray(): IArray

    getBuffer(): Buffer

    clear(): void
}

export interface IArray {
    dirty: boolean
    readonly length: number
    immutable: boolean

    JsArray(): Array<any>

    at(i: number): IObj

    pop(): IObj

    clear(): void

    push(v: Object): void

    write(destination: string): void
}

export type NPDFDictionaryKeyType = 'boolean' | 'long' | 'name' | 'real'

export interface IDictionary {
    dirty: boolean
    immutable: boolean

    tryGet(doc: IDocument, candidate: IObj): IDictionary | null

    /**
     * @param {string} k
     * @param {boolean} resolveValue - If value is a reference try to resolve the reference, defaults to true
     * @returns {IObj}
     */
    getKey(k: string, resolveValue?: boolean): IObj

    addKey(prop: string, value: boolean | number | string | IObj): void

    getKeys(): string[]

    hasKey(k: string): boolean

    removeKey(k: string): void

    getKeyAs(k: string, t: NPDFDictionaryKeyType): string | number

    clear(): void

    write(destination: string, cb: (e: Error, i: string) => void): void

    writeSync(destination: string): void
}

// export const resolveDictionary = (doc: Document, candidate: IObj): IDictionary | null => {
//     if (candidate instanceof (__mod.Obj as any))
//         return __mod.Dictionary.tryGetDictionary((doc as any)._instance, candidate)
//     else return null
// }
