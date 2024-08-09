#import "VGCoreAuto.tlb"     // 导入VGCoreAuto类型库: 这个文件在CorelDRAW 软件目录下可以找到
int main() {
  // 初始化COM库，使用多线程公寓模型
  HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);

  if (SUCCEEDED(hr)) {
    // 创建一个指向CorelDRAW应用程序的指针，经测试 CorelDRAW 16 可以正常绑定， 安装毛子版的 CorelDRAW 2020 无法正常绑定
    VGCore::IVGApplicationPtr app(L"CorelDRAW.Application.16");

    // 设置应用程序为可见状态
    app->Visible = VARIANT_TRUE;
    // 获取当前活动的文档，如果没有则创建一个新文档
    auto doc = app->ActiveDocument;
    if (!doc)
      doc = app->CreateDocument();

    // 清理COM库的初始化
    CoUninitialize();
  }

  return 0;
}