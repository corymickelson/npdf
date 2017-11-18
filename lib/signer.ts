import {__mod, Document} from './document'
import {SignatureField} from "./field";
import {existsSync} from "fs";

export class Signer {
    private _instance:any

    constructor(doc:Document, field: SignatureField, output?:string) {
        if(output && (output as any) instanceof String) {
            if(existsSync(output)) {
                throw Error("Do not write signed doc to the same path as the loaded document")
            } else {
                this._instance = new __mod.Signer(doc._instance, (field as any)._instance, output)
            }
        } else
            this._instance = new __mod.Signer(doc._instance, (field as any)._instance)
    }

    signWithCertificateAndKey(certfile:string, keyfile:string, password?: string): Buffer {
        return this._instance.signWithCertificateAndKey(certfile, keyfile, password || null)
    }

    sign(signature:Buffer): Buffer {
        return this._instance.sign(signature)
        // return new Promise((fulfill, reject) => {
            /*this._instance.sign((e:Error, d:Buffer|void) => {
                e ? reject(e) : fulfill(d)
            }, signature)*/
        // })
    }
}