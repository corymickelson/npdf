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

#include "SignatureField.h"
#include "../ErrorHandler.h"
#include "../ValidateArguments.h"
#include "../base/Data.h"
#include "../doc/Annotation.h"
#include "../doc/Document.h"
#include "../doc/Form.h"
#include "Signer.h"

namespace NoPoDoFo {

using namespace Napi;
using namespace PoDoFo;

FunctionReference SignatureField::constructor; // NOLINT

SignatureField::SignatureField(const CallbackInfo& info)
  : ObjectWrap<SignatureField>(info)
{
  try {
    if (info.Length() == 2) {
      auto annot = Annotation::Unwrap(info[0].As<Object>());
      doc = Document::Unwrap(info[1].As<Object>());
      field = new PdfSignatureField(&annot->GetAnnotation(),
                                    doc->GetDocument()->GetAcroForm(),
                                    doc->GetDocument());

    } else if (info.Length() == 1) {
      field = info[0].As<External<PdfSignatureField>>().Data();
    }
  } catch (PdfError& err) {
    ErrorHandler(err, info);
  }
}

void
SignatureField::Initialize(Napi::Env& env, Napi::Object& target)
{
  HandleScope scope(env);
  Function ctor = DefineClass(
    env,
    "SignatureField",
    { InstanceMethod("setAppearanceStream",
                     &SignatureField::SetAppearanceStream),
      InstanceMethod("setReason", &SignatureField::SetReason),
      InstanceMethod("setLocation", &SignatureField::SetLocation),
      InstanceMethod("setCreator", &SignatureField::SetCreator),
      InstanceMethod("setDate", &SignatureField::SetDate),
      InstanceMethod("addCertificateReference",
                     &SignatureField::AddCertificateReference),
      InstanceMethod("setFieldName", &SignatureField::SetFieldName),
      InstanceMethod("getObject", &SignatureField::GetSignatureObject),
      InstanceMethod("ensureSignatureObject",
                     &SignatureField::EnsureSignatureObject) });
  constructor = Napi::Persistent(ctor);
  constructor.SuppressDestruct();

  target.Set("SignatureField", ctor);
}

void
SignatureField::SetAppearanceStream(const CallbackInfo& info)
{
  throw Error::New(info.Env(), "unimplemented");
}

void
SignatureField::SetReason(const CallbackInfo& info)
{
  string reason = info[0].As<String>().Utf8Value();
  GetField()->SetSignatureReason(PdfString(reason));
}

void
SignatureField::SetLocation(const CallbackInfo& info)
{
  string location = info[0].As<String>().Utf8Value();
  GetField()->SetSignatureLocation(PdfString(location));
}

void
SignatureField::SetCreator(const CallbackInfo& info)
{
  string creator = info[0].As<String>().Utf8Value();
  GetField()->SetSignatureCreator(PdfName(creator));
}

void
SignatureField::SetDate(const CallbackInfo& info)
{
  if(info.Length() == 1 && info[0].IsString()) {
    GetField()->SetSignatureDate(PdfDate(PdfString(info[0].As<String>().Utf8Value())));
  } else {
    GetField()->SetSignatureDate(PdfDate());
  }
}

void
SignatureField::SetFieldName(const CallbackInfo& info)
{
  GetField()->SetFieldName(info[0].As<String>().Utf8Value());
}

void
SignatureField::AddCertificateReference(const CallbackInfo& info)
{
  auto flag = static_cast<PdfSignatureField::EPdfCertPermission>(info[0].As<Number>().Int32Value());
  GetField()->AddCertificationReference(doc->GetDocument()->GetCatalog(), flag);
}

Napi::Value
SignatureField::GetSignatureObject(const CallbackInfo& info)
{
  return External<PdfObject>::New(info.Env(), GetField()->GetSignatureObject());
}

Napi::Value
SignatureField::EnsureSignatureObject(const CallbackInfo& info)
{
  try {
    field->EnsureSignatureObject();
  } catch (PdfError& err) {
    ErrorHandler(err, info);
  }
  return info.Env().Undefined();
}
SignatureField::~SignatureField()
{
  HandleScope scope(Env());
  doc = nullptr;
  delete field;
  delete signatureBuffer;
}
}
