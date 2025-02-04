#include "iohook.h"
#include <iostream>

using namespace v8;
using Callback = Nan::Callback;
static bool sIsRuning=false; 

static HookProcessWorker* sIOHook = nullptr;

static void dispatch_proc(uiohook_event * const event) {
	if(sIOHook != nullptr && sIOHook->fHook != nullptr) {
		sIOHook->fHook->Send(event, sizeof(uiohook_event));
	}
}

static bool logger_proc(unsigned int level, const char *format, ...) {
	return true;
}

HookProcessWorker::HookProcessWorker(Nan::Callback* cb):
Nan::AsyncProgressWorkerBase<uiohook_event>(cb), fHook(nullptr) {}

void HookProcessWorker::Execute(const Nan::AsyncProgressWorkerBase<uiohook_event>::ExecutionProgress& progress) {
	hook_set_logger_proc(&logger_proc); hook_set_dispatch_proc(&dispatch_proc);
	fHook=&progress; hook_run();
}

void HookProcessWorker::Stop() {
	hook_stop(); sIsRuning=false;
}

void HookProcessWorker::HandleProgressCallback(const uiohook_event *data, size_t size) {
	HandleScope scope(Isolate::GetCurrent());
	Local<Object> obj = Nan::New<Object>();

	Nan::Set(obj, Nan::New("type").ToLocalChecked(), Nan::New((uint16_t)data->type));
	Nan::Set(obj, Nan::New("mask").ToLocalChecked(), Nan::New((uint16_t)data->mask));
	if((data->type >= EVENT_MOUSE_CLICKED) && (data->type < EVENT_MOUSE_WHEEL)) {
		Local<Object> mouse = Nan::New<Object>();
		Nan::Set(mouse, Nan::New("button").ToLocalChecked(), Nan::New((int)data->data.mouse.button));
		Nan::Set(mouse, Nan::New("clicks").ToLocalChecked(), Nan::New((int)data->data.mouse.clicks));
		Nan::Set(mouse, Nan::New("x").ToLocalChecked(), Nan::New((int)data->data.mouse.x));
		Nan::Set(mouse, Nan::New("y").ToLocalChecked(), Nan::New((int)data->data.mouse.y));
		Nan::Set(obj, Nan::New("mouse").ToLocalChecked(), mouse);
		Local<Value> argv[] = {obj}; callback->Call(1, argv);
	} else if((data->type >= EVENT_KEY_TYPED) && (data->type <= EVENT_KEY_RELEASED)) {
		Local<Object> kb = Nan::New<Object>();
		Nan::Set(kb, Nan::New("keychar").ToLocalChecked(), Nan::New((int)data->data.keyboard.keychar));
		Nan::Set(kb, Nan::New("keycode").ToLocalChecked(), Nan::New((int)data->data.keyboard.keycode));
		Nan::Set(kb, Nan::New("rawcode").ToLocalChecked(), Nan::New((int)data->data.keyboard.rawcode));
		Nan::Set(obj, Nan::New("kb").ToLocalChecked(), kb);
		Local<Value> argv[] = {obj}; callback->Call(1, argv);
	} else if(data->type == EVENT_MOUSE_WHEEL) {
		Local<Object> wheel = Nan::New<Object>();
		Nan::Set(wheel, Nan::New("amount").ToLocalChecked(), Nan::New((int)data->data.wheel.amount));
		Nan::Set(wheel, Nan::New("clicks").ToLocalChecked(), Nan::New((int)data->data.wheel.clicks));
		Nan::Set(wheel, Nan::New("direction").ToLocalChecked(), Nan::New((int)data->data.wheel.direction));
		Nan::Set(wheel, Nan::New("rotation").ToLocalChecked(), Nan::New((int)data->data.wheel.rotation));
		Nan::Set(wheel, Nan::New("type").ToLocalChecked(), Nan::New((int)data->data.wheel.type));
		Nan::Set(wheel, Nan::New("x").ToLocalChecked(), Nan::New((int)data->data.wheel.x));
		Nan::Set(wheel, Nan::New("y").ToLocalChecked(), Nan::New((int)data->data.wheel.y));
		Nan::Set(obj, Nan::New("wheel").ToLocalChecked(), wheel);
		Local<Value> argv[] = {obj}; callback->Call(1, argv);
	}
}

NAN_METHOD(StartHook) { //Allow one single execution
	if(sIsRuning==false) {
		if(info.Length() > 0) {
			if(info[0]->IsFunction()) {
				Callback* cb=new Callback(info[0].As<Function>());
				sIOHook=new HookProcessWorker(cb);
				Nan::AsyncQueueWorker(sIOHook); sIsRuning=true;
			}
		}
	}
}

NAN_METHOD(StopHook) { //Allow one single execution
	if((sIsRuning == true) && (sIOHook !=nullptr)) sIOHook->Stop();
}

NAN_MODULE_INIT(Init) {
	Nan::Set(target, Nan::New<String>("start_hook").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(StartHook)).ToLocalChecked());
	Nan::Set(target, Nan::New<String>("stop_hook").ToLocalChecked(),
		Nan::GetFunction(Nan::New<FunctionTemplate>(StopHook)).ToLocalChecked());
}

NODE_MODULE(nodeHook, Init)