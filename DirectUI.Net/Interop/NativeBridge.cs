using System;
using System.Runtime.InteropServices;

namespace DirectUI.Net.Interop
{
    internal static class NativeBridge
    {
        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        public delegate void ClickCallback(string elementId, IntPtr context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        public delegate void PropertyChangeCallback(string elementId, string propertyName, IntPtr context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        public delegate void SelectionCallback(string selectorId, string selectedId, int selectedIndex, IntPtr context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        public delegate void InputCallback(string elementId, int device, int code, int flags, IntPtr context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        public delegate void ParseErrorCallback(string error, string token, int line, IntPtr context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_Init(IntPtr hInst);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate void Fn_Shutdown();

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate IntPtr Fn_CreateWindow(IntPtr hwndParent, string title, int x, int y, int cx, int cy, uint style);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate void Fn_ShowWindow(IntPtr hwnd, int cmdShow);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate void Fn_DestroyWindow(IntPtr hwnd);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_LoadXml(IntPtr hwnd, string xml, [MarshalAs(UnmanagedType.LPWStr)] string resId);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_LoadXmlFile(IntPtr hwnd, string filePath, [MarshalAs(UnmanagedType.LPWStr)] string resId);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_MountFragment(IntPtr hwnd, string parentId, string xml, [MarshalAs(UnmanagedType.LPWStr)] string resId);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_MountFragmentFile(IntPtr hwnd, string parentId, string filePath, [MarshalAs(UnmanagedType.LPWStr)] string resId);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_RemoveElement(IntPtr hwnd, string id);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetText(IntPtr hwnd, string id, string text);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate IntPtr Fn_GetText(IntPtr hwnd, string id);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetEnabled(IntPtr hwnd, string id, [MarshalAs(UnmanagedType.Bool)] bool enabled);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_GetEnabled(IntPtr hwnd, string id);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetVisible(IntPtr hwnd, string id, [MarshalAs(UnmanagedType.Bool)] bool visible);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_GetVisible(IntPtr hwnd, string id);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetAlpha(IntPtr hwnd, string id, int alpha);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate int Fn_GetAlpha(IntPtr hwnd, string id);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetClass(IntPtr hwnd, string id, string className);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate IntPtr Fn_GetClass(IntPtr hwnd, string id);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetWidth(IntPtr hwnd, string id, int width);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate int Fn_GetWidth(IntPtr hwnd, string id);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetHeight(IntPtr hwnd, string id, int height);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate int Fn_GetHeight(IntPtr hwnd, string id);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetBackground(IntPtr hwnd, string id, uint color);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate uint Fn_GetBackground(IntPtr hwnd, string id);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetForeground(IntPtr hwnd, string id, uint color);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate uint Fn_GetForeground(IntPtr hwnd, string id);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetLayoutPos(IntPtr hwnd, string id, int layoutPos);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate int Fn_GetLayoutPos(IntPtr hwnd, string id);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_ElementExists(IntPtr hwnd, string id);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetStringProperty(IntPtr hwnd, string id, string propName, string value);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate IntPtr Fn_GetStringProperty(IntPtr hwnd, string id, string propName);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetIntProperty(IntPtr hwnd, string id, string propName, int value);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate int Fn_GetIntProperty(IntPtr hwnd, string id, string propName);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetBoolProperty(IntPtr hwnd, string id, string propName, [MarshalAs(UnmanagedType.Bool)] bool value);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_GetBoolProperty(IntPtr hwnd, string id, string propName);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetColorProperty(IntPtr hwnd, string id, string propName, uint color);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate uint Fn_GetColorProperty(IntPtr hwnd, string id, string propName);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetSelection(IntPtr hwnd, string selectorId, int index);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        private delegate int Fn_GetSelection(IntPtr hwnd, string selectorId);

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetSheet(IntPtr hwnd, string id, string sheetResId);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_StartDefer(IntPtr hwnd);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_EndDefer(IntPtr hwnd);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate void Fn_SetClickCallback(IntPtr hwnd, ClickCallback callback, IntPtr context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate void Fn_SetPropertyCallback(IntPtr hwnd, PropertyChangeCallback callback, IntPtr context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate void Fn_SetSelectionCallback(IntPtr hwnd, SelectionCallback callback, IntPtr context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate void Fn_SetInputCallback(IntPtr hwnd, InputCallback callback, IntPtr context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate void Fn_SetParseErrorCallback(ParseErrorCallback callback, IntPtr context);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate void Fn_RunMessagePump();

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate void Fn_StopMessagePump();

        [UnmanagedFunctionPointer(CallingConvention.StdCall, CharSet = CharSet.Unicode)]
        [return: MarshalAs(UnmanagedType.Bool)]
        private delegate bool Fn_SetContentBitmap(IntPtr hwnd, string id, IntPtr hBitmap);

        [UnmanagedFunctionPointer(CallingConvention.StdCall)]
        private delegate void Fn_FreeString(IntPtr str);

        private static readonly Fn_Init _Init = Load<Fn_Init>("DuiBridge_Init");
        private static readonly Fn_Shutdown _Shutdown = Load<Fn_Shutdown>("DuiBridge_Shutdown");
        private static readonly Fn_CreateWindow _CreateWindow = Load<Fn_CreateWindow>("DuiBridge_CreateWindow");
        private static readonly Fn_ShowWindow _ShowWindow = Load<Fn_ShowWindow>("DuiBridge_ShowWindow");
        private static readonly Fn_DestroyWindow _DestroyWindow = Load<Fn_DestroyWindow>("DuiBridge_DestroyWindow");
        private static readonly Fn_LoadXml _LoadXml = Load<Fn_LoadXml>("DuiBridge_LoadXml");
        private static readonly Fn_LoadXmlFile _LoadXmlFile = Load<Fn_LoadXmlFile>("DuiBridge_LoadXmlFile");
        private static readonly Fn_MountFragment _MountFragment = Load<Fn_MountFragment>("DuiBridge_MountFragment");
        private static readonly Fn_MountFragmentFile _MountFragmentFile = Load<Fn_MountFragmentFile>("DuiBridge_MountFragmentFile");
        private static readonly Fn_RemoveElement _RemoveElement = Load<Fn_RemoveElement>("DuiBridge_RemoveElement");
        private static readonly Fn_SetText _SetText = Load<Fn_SetText>("DuiBridge_SetText");
        private static readonly Fn_GetText _GetText = Load<Fn_GetText>("DuiBridge_GetText");
        private static readonly Fn_SetEnabled _SetEnabled = Load<Fn_SetEnabled>("DuiBridge_SetEnabled");
        private static readonly Fn_GetEnabled _GetEnabled = Load<Fn_GetEnabled>("DuiBridge_GetEnabled");
        private static readonly Fn_SetVisible _SetVisible = Load<Fn_SetVisible>("DuiBridge_SetVisible");
        private static readonly Fn_GetVisible _GetVisible = Load<Fn_GetVisible>("DuiBridge_GetVisible");
        private static readonly Fn_SetAlpha _SetAlpha = Load<Fn_SetAlpha>("DuiBridge_SetAlpha");
        private static readonly Fn_GetAlpha _GetAlpha = Load<Fn_GetAlpha>("DuiBridge_GetAlpha");
        private static readonly Fn_SetClass _SetClass = Load<Fn_SetClass>("DuiBridge_SetClass");
        private static readonly Fn_GetClass _GetClass = Load<Fn_GetClass>("DuiBridge_GetClass");
        private static readonly Fn_SetWidth _SetWidth = Load<Fn_SetWidth>("DuiBridge_SetWidth");
        private static readonly Fn_GetWidth _GetWidth = Load<Fn_GetWidth>("DuiBridge_GetWidth");
        private static readonly Fn_SetHeight _SetHeight = Load<Fn_SetHeight>("DuiBridge_SetHeight");
        private static readonly Fn_GetHeight _GetHeight = Load<Fn_GetHeight>("DuiBridge_GetHeight");
        private static readonly Fn_SetBackground _SetBackground = Load<Fn_SetBackground>("DuiBridge_SetBackground");
        private static readonly Fn_GetBackground _GetBackground = Load<Fn_GetBackground>("DuiBridge_GetBackground");
        private static readonly Fn_SetForeground _SetForeground = Load<Fn_SetForeground>("DuiBridge_SetForeground");
        private static readonly Fn_GetForeground _GetForeground = Load<Fn_GetForeground>("DuiBridge_GetForeground");
        private static readonly Fn_SetLayoutPos _SetLayoutPos = Load<Fn_SetLayoutPos>("DuiBridge_SetLayoutPos");
        private static readonly Fn_GetLayoutPos _GetLayoutPos = Load<Fn_GetLayoutPos>("DuiBridge_GetLayoutPos");
        private static readonly Fn_ElementExists _ElementExists = Load<Fn_ElementExists>("DuiBridge_ElementExists");
        private static readonly Fn_SetStringProperty _SetStringProp = Load<Fn_SetStringProperty>("DuiBridge_SetStringProperty");
        private static readonly Fn_GetStringProperty _GetStringProp = Load<Fn_GetStringProperty>("DuiBridge_GetStringProperty");
        private static readonly Fn_SetIntProperty _SetIntProp = Load<Fn_SetIntProperty>("DuiBridge_SetIntProperty");
        private static readonly Fn_GetIntProperty _GetIntProp = Load<Fn_GetIntProperty>("DuiBridge_GetIntProperty");
        private static readonly Fn_SetBoolProperty _SetBoolProp = Load<Fn_SetBoolProperty>("DuiBridge_SetBoolProperty");
        private static readonly Fn_GetBoolProperty _GetBoolProp = Load<Fn_GetBoolProperty>("DuiBridge_GetBoolProperty");
        private static readonly Fn_SetColorProperty _SetColorProp = Load<Fn_SetColorProperty>("DuiBridge_SetColorProperty");
        private static readonly Fn_GetColorProperty _GetColorProp = Load<Fn_GetColorProperty>("DuiBridge_GetColorProperty");
        private static readonly Fn_SetSelection _SetSelection = Load<Fn_SetSelection>("DuiBridge_SetSelection");
        private static readonly Fn_GetSelection _GetSelection = Load<Fn_GetSelection>("DuiBridge_GetSelection");
        private static readonly Fn_SetSheet _SetSheet = Load<Fn_SetSheet>("DuiBridge_SetSheet");
        private static readonly Fn_StartDefer _StartDefer = Load<Fn_StartDefer>("DuiBridge_StartDefer");
        private static readonly Fn_EndDefer _EndDefer = Load<Fn_EndDefer>("DuiBridge_EndDefer");
        private static readonly Fn_SetClickCallback _SetClickCb = Load<Fn_SetClickCallback>("DuiBridge_SetClickCallback");
        private static readonly Fn_SetPropertyCallback _SetPropCb = Load<Fn_SetPropertyCallback>("DuiBridge_SetPropertyCallback");
        private static readonly Fn_SetSelectionCallback _SetSelCb = Load<Fn_SetSelectionCallback>("DuiBridge_SetSelectionCallback");
        private static readonly Fn_SetInputCallback _SetInputCb = Load<Fn_SetInputCallback>("DuiBridge_SetInputCallback");
        private static readonly Fn_SetParseErrorCallback _SetParseCb = Load<Fn_SetParseErrorCallback>("DuiBridge_SetParseErrorCallback");
        private static readonly Fn_RunMessagePump _RunMessagePump = Load<Fn_RunMessagePump>("DuiBridge_RunMessagePump");
        private static readonly Fn_StopMessagePump _StopMessagePump = Load<Fn_StopMessagePump>("DuiBridge_StopMessagePump");
        private static readonly Fn_SetContentBitmap _SetContentBitmap = Load<Fn_SetContentBitmap>("DuiBridge_SetContentBitmap");
        private static readonly Fn_FreeString _FreeString = Load<Fn_FreeString>("DuiBridge_FreeString");

        public static bool DuiBridge_Init(IntPtr hInst) => _Init?.Invoke(hInst) ?? false;
        public static void DuiBridge_Shutdown() => _Shutdown?.Invoke();
        public static IntPtr DuiBridge_CreateWindow(IntPtr hwndParent, string title, int x, int y, int cx, int cy, uint style) => _CreateWindow?.Invoke(hwndParent, title, x, y, cx, cy, style) ?? IntPtr.Zero;
        public static void DuiBridge_ShowWindow(IntPtr hwnd, int cmdShow) => _ShowWindow?.Invoke(hwnd, cmdShow);
        public static void DuiBridge_DestroyWindow(IntPtr hwnd) => _DestroyWindow?.Invoke(hwnd);
        public static bool DuiBridge_LoadXml(IntPtr hwnd, string xml, string resId) => _LoadXml?.Invoke(hwnd, xml, resId) ?? false;
        public static bool DuiBridge_LoadXmlFile(IntPtr hwnd, string filePath, string resId) => _LoadXmlFile?.Invoke(hwnd, filePath, resId) ?? false;
        public static bool DuiBridge_MountFragment(IntPtr hwnd, string parentId, string xml, string resId) => _MountFragment?.Invoke(hwnd, parentId, xml, resId) ?? false;
        public static bool DuiBridge_MountFragmentFile(IntPtr hwnd, string parentId, string filePath, string resId) => _MountFragmentFile?.Invoke(hwnd, parentId, filePath, resId) ?? false;
        public static bool DuiBridge_RemoveElement(IntPtr hwnd, string id) => _RemoveElement?.Invoke(hwnd, id) ?? false;
        public static bool DuiBridge_SetText(IntPtr hwnd, string id, string text) => _SetText?.Invoke(hwnd, id, text) ?? false;
        public static IntPtr DuiBridge_GetText(IntPtr hwnd, string id) => _GetText?.Invoke(hwnd, id) ?? IntPtr.Zero;
        public static bool DuiBridge_SetEnabled(IntPtr hwnd, string id, bool enabled) => _SetEnabled?.Invoke(hwnd, id, enabled) ?? false;
        public static bool DuiBridge_GetEnabled(IntPtr hwnd, string id) => _GetEnabled?.Invoke(hwnd, id) ?? false;
        public static bool DuiBridge_SetVisible(IntPtr hwnd, string id, bool visible) => _SetVisible?.Invoke(hwnd, id, visible) ?? false;
        public static bool DuiBridge_GetVisible(IntPtr hwnd, string id) => _GetVisible?.Invoke(hwnd, id) ?? false;
        public static bool DuiBridge_SetAlpha(IntPtr hwnd, string id, int alpha) => _SetAlpha?.Invoke(hwnd, id, alpha) ?? false;
        public static int DuiBridge_GetAlpha(IntPtr hwnd, string id) => _GetAlpha?.Invoke(hwnd, id) ?? 255;
        public static bool DuiBridge_SetClass(IntPtr hwnd, string id, string className) => _SetClass?.Invoke(hwnd, id, className) ?? false;
        public static IntPtr DuiBridge_GetClass(IntPtr hwnd, string id) => _GetClass?.Invoke(hwnd, id) ?? IntPtr.Zero;
        public static bool DuiBridge_SetWidth(IntPtr hwnd, string id, int width) => _SetWidth?.Invoke(hwnd, id, width) ?? false;
        public static int DuiBridge_GetWidth(IntPtr hwnd, string id) => _GetWidth?.Invoke(hwnd, id) ?? 0;
        public static bool DuiBridge_SetHeight(IntPtr hwnd, string id, int height) => _SetHeight?.Invoke(hwnd, id, height) ?? false;
        public static int DuiBridge_GetHeight(IntPtr hwnd, string id) => _GetHeight?.Invoke(hwnd, id) ?? 0;
        public static bool DuiBridge_SetBackground(IntPtr hwnd, string id, uint color) => _SetBackground?.Invoke(hwnd, id, color) ?? false;
        public static uint DuiBridge_GetBackground(IntPtr hwnd, string id) => _GetBackground?.Invoke(hwnd, id) ?? 0;
        public static bool DuiBridge_SetForeground(IntPtr hwnd, string id, uint color) => _SetForeground?.Invoke(hwnd, id, color) ?? false;
        public static uint DuiBridge_GetForeground(IntPtr hwnd, string id) => _GetForeground?.Invoke(hwnd, id) ?? 0;
        public static bool DuiBridge_SetLayoutPos(IntPtr hwnd, string id, int layoutPos) => _SetLayoutPos?.Invoke(hwnd, id, layoutPos) ?? false;
        public static int DuiBridge_GetLayoutPos(IntPtr hwnd, string id) => _GetLayoutPos?.Invoke(hwnd, id) ?? 0;
        public static bool DuiBridge_ElementExists(IntPtr hwnd, string id) => _ElementExists?.Invoke(hwnd, id) ?? false;
        public static bool DuiBridge_SetStringProperty(IntPtr hwnd, string id, string propName, string value) => _SetStringProp?.Invoke(hwnd, id, propName, value) ?? false;
        public static IntPtr DuiBridge_GetStringProperty(IntPtr hwnd, string id, string propName) => _GetStringProp?.Invoke(hwnd, id, propName) ?? IntPtr.Zero;
        public static bool DuiBridge_SetIntProperty(IntPtr hwnd, string id, string propName, int value) => _SetIntProp?.Invoke(hwnd, id, propName, value) ?? false;
        public static int DuiBridge_GetIntProperty(IntPtr hwnd, string id, string propName) => _GetIntProp?.Invoke(hwnd, id, propName) ?? 0;
        public static bool DuiBridge_SetBoolProperty(IntPtr hwnd, string id, string propName, bool value) => _SetBoolProp?.Invoke(hwnd, id, propName, value) ?? false;
        public static bool DuiBridge_GetBoolProperty(IntPtr hwnd, string id, string propName) => _GetBoolProp?.Invoke(hwnd, id, propName) ?? false;
        public static bool DuiBridge_SetColorProperty(IntPtr hwnd, string id, string propName, uint color) => _SetColorProp?.Invoke(hwnd, id, propName, color) ?? false;
        public static uint DuiBridge_GetColorProperty(IntPtr hwnd, string id, string propName) => _GetColorProp?.Invoke(hwnd, id, propName) ?? 0;
        public static bool DuiBridge_SetSelection(IntPtr hwnd, string selectorId, int index) => _SetSelection?.Invoke(hwnd, selectorId, index) ?? false;
        public static int DuiBridge_GetSelection(IntPtr hwnd, string selectorId) => _GetSelection?.Invoke(hwnd, selectorId) ?? -1;
        public static bool DuiBridge_SetSheet(IntPtr hwnd, string id, string sheetResId) => _SetSheet?.Invoke(hwnd, id, sheetResId) ?? false;
        public static bool DuiBridge_StartDefer(IntPtr hwnd) => _StartDefer?.Invoke(hwnd) ?? false;
        public static bool DuiBridge_EndDefer(IntPtr hwnd) => _EndDefer?.Invoke(hwnd) ?? false;
        public static void DuiBridge_SetClickCallback(IntPtr hwnd, ClickCallback callback, IntPtr context) => _SetClickCb?.Invoke(hwnd, callback, context);
        public static void DuiBridge_SetPropertyCallback(IntPtr hwnd, PropertyChangeCallback callback, IntPtr context) => _SetPropCb?.Invoke(hwnd, callback, context);
        public static void DuiBridge_SetSelectionCallback(IntPtr hwnd, SelectionCallback callback, IntPtr context) => _SetSelCb?.Invoke(hwnd, callback, context);
        public static void DuiBridge_SetInputCallback(IntPtr hwnd, InputCallback callback, IntPtr context) => _SetInputCb?.Invoke(hwnd, callback, context);
        public static void DuiBridge_SetParseErrorCallback(ParseErrorCallback callback, IntPtr context) => _SetParseCb?.Invoke(callback, context);
        public static void DuiBridge_RunMessagePump() => _RunMessagePump?.Invoke();
        public static void DuiBridge_StopMessagePump() => _StopMessagePump?.Invoke();
        public static bool DuiBridge_SetContentBitmap(IntPtr hwnd, string id, IntPtr hBitmap) => _SetContentBitmap?.Invoke(hwnd, id, hBitmap) ?? false;
        public static void DuiBridge_FreeString(IntPtr str) => _FreeString?.Invoke(str);

        private static T Load<T>(string name) where T : class
        {
            IntPtr hMod = GetModuleHandleW("dui_bridge.dll");
            if (hMod == IntPtr.Zero) return null;

            IntPtr proc = GetProcAddress(hMod, name);
            if (proc == IntPtr.Zero) return null;

            return Marshal.GetDelegateForFunctionPointer(proc, typeof(T)) as T;
        }

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        private static extern IntPtr GetModuleHandleW(string lpModuleName);

        [DllImport("kernel32.dll", CharSet = CharSet.Ansi, BestFitMapping = false, SetLastError = true)]
        private static extern IntPtr GetProcAddress(IntPtr hModule, string procName);
    }
}