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

/**
 * Indirect object reference [GenerationNumber, ObjectNumber]
 */
export type Ref = [number, number]

export interface IArray {
    dirty: boolean
    readonly length: number
    immutable: boolean

    // JsArray(): Array<any>

    /**
     * If the item at the index is a Reference that can not be resolved by the array object owner.
     * A Ref will be returned, the Ref can be resolved using getObject on the document itself.
     * @todo Fix null owner when trying to resolve Reference type
     * @param i
     */
    at(i: number): Ref | IObj

    pop(): IObj

    clear(): void

    push(v: Object): void

    write(destination: string): void
}

export type NPDFDictionaryKeyType = 'boolean' | 'long' | 'name' | 'real'

export interface IDictionary {
    dirty: boolean
    immutable: boolean

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
