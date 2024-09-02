#include "cdrapp.h"
#include <stdio.h>
#include <windows.h>
#include <thread>
#include <chrono>



#define CUSTOM_FORMAT RegisterClipboardFormatA("Portable Document Format")

bool pdf_to_clipboard(const char *pdffile)
{
    // 打开剪贴板
    if (!OpenClipboard(NULL)) {
        // printf("Failed to open clipboard.\n");
        return false;
    }

    // 清空剪贴板
    EmptyClipboard();

    // 读取PDF文件到内存
    FILE *file = fopen(pdffile, "rb");
    if (!file) {
        // printf("Failed to open file.\n");
        CloseClipboard();
        return false;
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    size_t fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);

    // 分配内存并读取文件内容
    void *pdfData = malloc(fileSize);
    if (!pdfData) {
        // printf("Failed to allocate memory.\n");
        fclose(file);
        CloseClipboard();
        return false;
    }

    fread(pdfData, 1, fileSize, file);
    fclose(file);

    // 将二进制数据写入剪贴板
    HGLOBAL hGlobal = GlobalAlloc(GHND, fileSize);
    if (!hGlobal) {
        // printf("Failed to allocate global memory.\n");
        free(pdfData);
        CloseClipboard();
        return false;
    }

    memcpy(GlobalLock(hGlobal), pdfData, fileSize);
    GlobalUnlock(hGlobal);

    if (!SetClipboardData(CUSTOM_FORMAT, hGlobal)) {
        // printf("Failed to set clipboard data.\n");
        GlobalFree(hGlobal);
        free(pdfData);
        CloseClipboard();
        return false;
    }
    // 关闭剪贴板
    CloseClipboard();

    // printf("PDF binary data copied to clipboard using custom format.\n");
    // 不要忘记释放内存
    free(pdfData);
    return true;
}

bool clipboard_to_pdf(const char *outputFile)
{
    // 打开剪贴板
    if (!OpenClipboard(NULL)) {
        // printf("Failed to open clipboard.\n");
        return false;
    }

    // 获取剪贴板中的PDF数据
    HANDLE hData = GetClipboardData(CUSTOM_FORMAT);
    if (!hData) {
        // printf("Failed to get clipboard data.\n");
        CloseClipboard();
        return false;
    }

    // 锁定内存并获取指针
    void *pdfData = GlobalLock(hData);
    if (!pdfData) {
        // printf("Failed to lock global memory.\n");
        CloseClipboard();
        return false;
    }

    // 获取PDF数据的大小
    size_t fileSize = GlobalSize(hData);

    // 将PDF数据写入文件
    FILE *file = fopen(outputFile, "wb");
    if (!file) {
        // printf("Failed to open output file.\n");
        GlobalUnlock(hData);
        CloseClipboard();
        return false;
    }

    fwrite(pdfData, 1, fileSize, file);
    fclose(file);

    // 解锁内存并关闭剪贴板
    GlobalUnlock(hData);
    CloseClipboard();

    // printf("PDF binary data from clipboard saved to file: %s\n", outputFile);

    return true;
}

bool cdr_savepdf(corel *cdr, const char *outputFile)
{
    DeleteFile(_bstr_t(outputFile));
    auto pdfst = cdr->ActiveDocument->PDFSettings;
    pdfst->BitmapCompression = pdfLZW;
    pdfst->ColorMode = pdfCMYK;
    pdfst->EmbedBaseFonts = cdrFalse;
    pdfst->EmbedFonts = cdrFalse;
    pdfst->FileInformation = cdrFalse;
    pdfst->Hyperlinks = cdrFalse;
    pdfst->IncludeBleed = cdrFalse;
    pdfst->Linearize = cdrTrue;
    pdfst->MaintainOPILinks = cdrTrue;
    pdfst->Overprints = cdrTrue;
    pdfst->PutpdfVersion(pdfVersion14); //'pdfVersion14 : pdfVersion13;
    pdfst->PublishRange = pdfSelection;
    pdfst->RegistrationMarks = cdrFalse;
    pdfst->SpotColors = cdrTrue;
    pdfst->Startup = pdfPageOnly;
    pdfst->SubsetFonts = cdrFalse;
    pdfst->TextAsCurves = cdrFalse;
    pdfst->Thumbnails = cdrFalse;
    cdr->ActiveDocument->PublishToPDF(_bstr_t(outputFile));

    return true;
}

void CdrCopy_to_AdobeAI(corel *cdr)
{
    char path[MAX_PATH] = {0};
    GetTempPath(MAX_PATH, path);
    char *f = strcat(path, "CDR2AI.pdf");
    if (cdr_savepdf(cdr, f))
        pdf_to_clipboard(f);
}

bool pdf_ImportCdr(corel *cdr, const char *pdffile)
{
   auto si = cdr->CreateStructImportOptions();
   si->MaintainLayers = true;
   
   auto impflt = cdr->ActiveLayer->ImportEx(_bstr_t(pdffile), cdrPDF , si);
   impflt->Finish();
   return true;
}

void AdobeAI_Copy_ImportCdr(corel *cdr)
{
    char path[MAX_PATH] = {0};
    GetTempPath(MAX_PATH, path);
    char *f = strcat(path, "CDR2AI.pdf");
    if (clipboard_to_pdf(f)){
        // 延时 0.5 秒
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        pdf_ImportCdr(cdr, f);
    }

}