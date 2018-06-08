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

#include "Page.h"
#include "../ErrorHandler.h"
#include "../base/Obj.h"
#include "Annotation.h"
#include "CheckBox.h"
#include "ComboBox.h"
#include "Field.h"
#include "ListBox.h"
#include "PushButton.h"
#include "SignatureField.h"
#include "TextField.h"

using namespace Napi;
using namespace PoDoFo;

using std::cout;
using std::endl;
using std::make_shared;
using std::stringstream;

namespace NoPoDoFo {

FunctionReference Page::constructor; // NOLINT

Page::Page(const CallbackInfo& info)
  : ObjectWrap(info)
  , page(info[0].As<External<PdfPage>>().Data())
{
  // The page is managed by the document. Do not pass a page pointer to this
  // class without first adding it to the document
}

Page::~Page()
{
  HandleScope scope(Env());
  page = nullptr;
}

void
Page::Initialize(Napi::Env& env, Napi::Object& target)
{
  HandleScope scope(env);
  Function ctor = DefineClass(
    env,
    "Page",
    { InstanceAccessor("rotation", &Page::GetRotation, &Page::SetRotation),
      InstanceAccessor("trimBox", &Page::GetTrimBox, &Page::SetTrimBox),
      InstanceAccessor("number", &Page::GetPageNumber, nullptr),
      InstanceAccessor("width", &Page::GetPageWidth, &Page::SetPageWidth),
      InstanceAccessor("height", &Page::GetPageHeight, &Page::SetPageHeight),
      InstanceAccessor("contents", &Page::GetContents, nullptr),
      InstanceAccessor("resources", &Page::GetResources, nullptr),

      InstanceMethod("getField", &Page::GetField),
      InstanceMethod("getFields", &Page::GetFields),
      InstanceMethod("fieldsCount", &Page::GetNumFields),
      InstanceMethod("getFieldIndex", &Page::GetFieldIndex),
      InstanceMethod("getMediaBox", &Page::GetMediaBox),
      InstanceMethod("getBleedBox", &Page::GetBleedBox),
      InstanceMethod("getArtBox", &Page::GetArtBox),
      InstanceMethod("createAnnotation", &Page::CreateAnnotation),
      InstanceMethod("getAnnotation", &Page::GetAnnotation),
      InstanceMethod("annotationsCount", &Page::GetNumAnnots),
      InstanceMethod("deleteAnnotation", &Page::DeleteAnnotation) });
  constructor = Persistent(ctor);
  constructor.SuppressDestruct();
  target.Set("Page", ctor);
}
Napi::Value
Page::GetRotation(const CallbackInfo& info)
{
  return Number::New(info.Env(), page->GetRotation());
}
Napi::Value
Page::GetNumFields(const CallbackInfo& info)
{
  return Number::New(info.Env(), page->GetNumFields());
}

Napi::Value
Page::GetField(const CallbackInfo& info)
{
  int index = info[0].As<Number>();
  if (page->GetNumFields() < index || index < 0) {
    RangeError::New(info.Env(), "index out of range")
      .ThrowAsJavaScriptException();
    return info.Env().Undefined();
  }
  return GetField(info.Env(), index);
}
Napi::Value
Page::GetField(const Napi::Env& env, int index)
{
  auto field = page->GetField(index);
  EPdfField t = field.GetType();
  switch (t) {
    case ePdfField_PushButton:
      return PushButton::constructor.New(
        { External<PdfField>::New(env, &field) });
    case ePdfField_CheckBox:
      return CheckBox::constructor.New(
        { External<PdfField>::New(env, &field) });
    case ePdfField_RadioButton:
      Error::New(env, "RadioButton not yet implemented")
        .ThrowAsJavaScriptException();
      return env.Undefined();
    case ePdfField_TextField:
      return TextField::constructor.New(
        { External<PdfField>::New(env, &field) });
    case ePdfField_ComboBox:
      return ComboBox::constructor.New(
        { External<PdfField>::New(env, &field) });
    case ePdfField_ListBox:
      return ListBox::constructor.New({ External<PdfField>::New(env, &field) });
    case ePdfField_Signature:
      return SignatureField::constructor.New(
        { External<PdfAnnotation>::New(env, field.GetWidgetAnnotation()) });
    default:
      Error::New(env, "Unknown field type").ThrowAsJavaScriptException();
      return env.Undefined();
  }
}

Napi::Value
Page::GetFields(const CallbackInfo& info)
{
  auto js = Array::New(info.Env());
  uint32_t n = 0;
  for (auto i = 0; i < page->GetNumFields(); ++i) {
    js.Set(n, GetField(info.Env(), i));
    ++n;
  }
  return js;
}

Napi::Object
Page::ExtractAndApplyRectValues(const CallbackInfo& info, PdfRect& rect)
{
  Napi::Object obj = Napi::Object::New(info.Env());
  obj.Set("left", Napi::Number::New(info.Env(), rect.GetLeft()));
  obj.Set("bottom", Napi::Number::New(info.Env(), rect.GetBottom()));
  obj.Set("width", Napi::Number::New(info.Env(), rect.GetWidth()));
  obj.Set("height", Napi::Number::New(info.Env(), rect.GetHeight()));
  return obj;
}

void
Page::SetRotation(const CallbackInfo& info, const Napi::Value& value)
{
  if (!value.IsNumber()) {
    throw Napi::Error::New(info.Env(), "Rotation must be number");
  }
  int rotate = value.As<Number>();
  if (rotate < 0 || rotate > 270) {
    throw Napi::Error::New(info.Env(),
                           "Rotate values must be a value of: 0, 90, 180, 270");
  }
  page->SetRotation(rotate);
}

Napi::Value
Page::GetFieldIndex(const CallbackInfo& info)
{
  string key = info[0].As<String>().Utf8Value();
  int index = -1;
  for (int i = 0; i < page->GetNumFields(); ++i) {
    PdfField field = page->GetField(i);
    string name = field.GetFieldName().GetStringUtf8();
    string alternate = field.GetAlternateName().GetStringUtf8();
    string mapping = field.GetMappingName().GetStringUtf8();
    if (key == name || key == alternate || key == mapping) {
      index = i;
      break;
    }
  }
  return Number::New(info.Env(), index);
}

void
Page::SetPageWidth(const CallbackInfo& info, const Napi::Value& value)
{
  if (!value.IsNumber()) {
    throw Napi::Error::New(info.Env(), "Page width must be of type number");
  }
  try {
    int width = value.As<Number>();
    page->SetPageWidth(width);
  } catch (PdfError& err) {
    ErrorHandler(err, info);
  }
}

void
Page::SetPageHeight(const CallbackInfo& info, const Napi::Value& value)
{
  if (!value.IsNumber()) {
    throw Napi::Error::New(info.Env(), "Page height must be of type number");
  }
  try {
    int height = value.As<Number>();
    page->SetPageWidth(height);
  } catch (PdfError& err) {
    ErrorHandler(err, info);
  }
}

Napi::Value
Page::GetPageWidth(const CallbackInfo& info)
{
  return Napi::Number::New(info.Env(), page->GetPageSize().GetWidth());
}

Napi::Value
Page::GetPageHeight(const CallbackInfo& info)
{
  return Napi::Number::New(info.Env(), page->GetPageSize().GetHeight());
}

Napi::Value
Page::GetTrimBox(const CallbackInfo& info)
{
  PdfRect trimBox = page->GetTrimBox();
  return ExtractAndApplyRectValues(info, trimBox);
}

void
Page::SetTrimBox(const CallbackInfo& info, const Napi::Value& value)
{
  if (!value.IsObject()) {
    throw Napi::Error::New(info.Env(), "trimbox must be an instance of Rect");
  }
  try {
    Rect* rect = Rect::Unwrap(value.As<Object>());
    auto pdfRect = rect->GetRect();
    page->SetTrimBox(*pdfRect);

  } catch (PdfError& err) {
    ErrorHandler(err, info);
  }
}

Napi::Value
Page::GetPageNumber(const CallbackInfo& info)
{
  return Napi::Number::New(info.Env(), page->GetPageNumber());
}

Napi::Value
Page::GetContents(const CallbackInfo& info)
{
  EscapableHandleScope scope(info.Env());
  // bool forAppending = info[0].As<Boolean>();
  PdfObject* contentsObj = page->GetContents();
  // forAppending ? page->GetContentsForAppending() :
  // page->GetContents();
  auto objPtr = External<PdfObject>::New(info.Env(), contentsObj);
  auto instance = Obj::constructor.New({ objPtr });
  return instance;
}

Napi::Value
Page::GetResources(const CallbackInfo& info)
{
  EscapableHandleScope scope(info.Env());
  PdfObject* resources = page->GetResources();
  auto objPtr = External<PdfObject>::New(info.Env(), resources);
  auto instance = Obj::constructor.New({ objPtr });
  return instance;
}

Napi::Value
Page::GetMediaBox(const CallbackInfo& info)
{
  PdfRect mediaBox = page->GetMediaBox();
  return ExtractAndApplyRectValues(info, mediaBox);
}

Napi::Value
Page::GetBleedBox(const CallbackInfo& info)
{
  PdfRect bleedBox = page->GetBleedBox();
  return ExtractAndApplyRectValues(info, bleedBox);
}

Napi::Value
Page::GetArtBox(const CallbackInfo& info)
{
  PdfRect artBox = page->GetArtBox();
  return ExtractAndApplyRectValues(info, artBox);
}

Napi::Value
Page::GetNumAnnots(const CallbackInfo& info)
{
  return Napi::Number::New(info.Env(), page->GetNumAnnots());
}

void
Page::DeleteAnnotation(const CallbackInfo& info)
{
  int index = info[0].As<Number>();
  try {
    page->DeleteAnnotation(index);
  } catch (PdfError& err) {
    ErrorHandler(err, info);
  }
}
Napi::Value
Page::GetAnnotation(const CallbackInfo& info)
{
  EscapableHandleScope scope(info.Env());
  int index = info[0].As<Number>();
  auto ptr = page->GetAnnotation(index);
  auto instance = External<PdfAnnotation>::New(info.Env(), ptr);
  return Annotation::constructor.New({ instance });
}
Napi::Value
Page::CreateAnnotation(const CallbackInfo& info)
{
  EscapableHandleScope scope(info.Env());
  int flag = info[0].As<Number>();
  auto type = static_cast<EPdfAnnotation>(flag);
  auto obj = info[1].As<Object>();
  Rect* rect = Rect::Unwrap(obj);
  PdfAnnotation* annot = page->CreateAnnotation(type, *rect->GetRect());
  auto instance = Annotation::constructor.New(
    { External<PdfAnnotation>::New(info.Env(), annot) });
  return instance;
}
}
