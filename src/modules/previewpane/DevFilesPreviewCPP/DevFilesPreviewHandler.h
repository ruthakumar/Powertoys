#pragma once

#include "pch.h"

#include <filesystem>
#include <ShlObj.h>
#include <string>

#include <WebView2.h>
#include <wil/com.h>

class DevFilesPreviewHandler :
    public IInitializeWithFile,
    public IPreviewHandler,
    public IPreviewHandlerVisuals,
    public IOleWindow,
    public IObjectWithSite
{
public:
    // IUnknown
    IFACEMETHODIMP QueryInterface(REFIID riid, void** ppv);
    IFACEMETHODIMP_(ULONG) AddRef();
    IFACEMETHODIMP_(ULONG) Release();

    // IInitializeWithStream
    IFACEMETHODIMP Initialize(LPCWSTR pszFilePath, DWORD grfMode);

    // IPreviewHandler
    IFACEMETHODIMP SetWindow(HWND hwnd, const RECT* prc);
    IFACEMETHODIMP SetFocus();
    IFACEMETHODIMP QueryFocus(HWND* phwnd);
    IFACEMETHODIMP TranslateAccelerator(MSG* pmsg);
    IFACEMETHODIMP SetRect(const RECT* prc);
    IFACEMETHODIMP DoPreview();
    IFACEMETHODIMP Unload();

    // IPreviewHandlerVisuals
    IFACEMETHODIMP SetBackgroundColor(COLORREF color);
    IFACEMETHODIMP SetFont(const LOGFONTW* plf);
    IFACEMETHODIMP SetTextColor(COLORREF color);

    // IOleWindow
    IFACEMETHODIMP GetWindow(HWND* phwnd);
    IFACEMETHODIMP ContextSensitiveHelp(BOOL fEnterMode);

    // IObjectWithSite
    IFACEMETHODIMP SetSite(IUnknown* punkSite);
    IFACEMETHODIMP GetSite(REFIID riid, void** ppv);

    DevFilesPreviewHandler();

protected:
    ~DevFilesPreviewHandler();

private:
    // Reference count of component.
    long m_cRef;

    // Provided during initialization.
    std::wstring m_filePath;

    // Parent window that hosts the previewer window.
    // Note: do NOT DestroyWindow this.
    HWND m_hwndParent;
    // Bounding rect of the parent window.
    RECT m_rcParent;

    // Site pointer from host, used to get IPreviewHandlerFrame.
    IUnknown* m_punkSite;

    std::filesystem::path m_webVew2UserDataFolder;

    // Pointer to WebViewController
    wil::com_ptr<ICoreWebView2Controller> m_webviewController;

    // Pointer to WebView window
    wil::com_ptr<ICoreWebView2> m_webviewWindow;

    HWND m_gpoText;
    HWND m_errorText;
    bool m_infoBarAdded;

    std::wstring GetLanguage(std::wstring fileExtension);
    void PreviewError(std::wstring errorMessage);
    void CleanupWebView2UserDataFolder();
};