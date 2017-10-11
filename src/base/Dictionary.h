//
// Created by red on 9/23/17.
//

#include <napi.h>
#include <podofo/podofo.h>
#include <string>

using namespace std;

class Dictionary : public Napi::ObjectWrap<Dictionary>
{
public:
  explicit Dictionary(const Napi::CallbackInfo&);
  ~Dictionary() { delete _obj; }
  static void Initialize(Napi::Env& env, Napi::Object& target);

  void AddKey(const Napi::CallbackInfo&);
  Napi::Value GetKey(const Napi::CallbackInfo&);
  Napi::Value GetKeys(const Napi::CallbackInfo&);
  Napi::Value RemoveKey(const Napi::CallbackInfo&);
  Napi::Value HasKey(const Napi::CallbackInfo&);
  Napi::Value Clear(const Napi::CallbackInfo&);
  void SetImmutable(const Napi::CallbackInfo&, const Napi::Value&);
  Napi::Value GetImmutable(const Napi::CallbackInfo&);
  void SetDirty(const Napi::CallbackInfo&, const Napi::Value&);
  Napi::Value GetDirty(const Napi::CallbackInfo&);
  Napi::Value GetKeyAs(const Napi::CallbackInfo&);
  void Write(const Napi::CallbackInfo&);
  void WriteSync(const Napi::CallbackInfo&);
  Napi::Value ToObject(const Napi::CallbackInfo&);

  PoDoFo::PdfDictionary* GetDictionary() { return &dict; }

private:
  const PoDoFo::PdfObject* _obj;
  PoDoFo::PdfDictionary dict = PoDoFo::PdfDictionary();
};

class DictWriteAsync : public Napi::AsyncWorker
{
public:
  DictWriteAsync(Napi::Function& cb, Dictionary* dict, string dest)
    : Napi::AsyncWorker(cb)
    , dict(dict)
    , arg(std::move(dest))
  {}
  ~DictWriteAsync() {}

protected:
  void Execute();
  void OnOK();

private:
  Dictionary* dict;
  string arg;
  const char* eMessage = nullptr;
};
