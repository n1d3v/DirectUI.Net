using DirectUI.Net.Events;
using DirectUI.Net.Interop;
using System;
using System.Diagnostics;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Threading;

namespace DirectUI.Net.Core
{
    public class DuiWindow : IDisposable
    {
        private IntPtr _hwnd;
        private bool _disposed;

        private readonly NativeBridge.ClickCallback _clickDelegate;
        private readonly NativeBridge.PropertyChangeCallback _propDelegate;
        private readonly NativeBridge.SelectionCallback _selDelegate;
        private readonly NativeBridge.InputCallback _inputDelegate;

        private readonly SynchronizationContext _syncContext;

        private static readonly long ClickDebounceThreshold = Stopwatch.Frequency * 350 / 1000;
        private readonly object _clickLock = new object();
        private long _lastClickTick;

        public IntPtr Handle => _hwnd;

        // All events available in DirectUI.Net
        public event EventHandler<DuiClickEventArgs> ButtonClicked;
        public event EventHandler<DuiPropertyEventArgs> PropertyChanged;
        public event EventHandler<DuiSelectionEventArgs> SelectionChanged;
        public event EventHandler<DuiInputEventArgs> Input;

        public DuiWindow(string title, int x, int y, int width, int height, IntPtr parentHwnd = default, uint style = 0)
        {
            if (!DuiInitializer.Instance.Initialized)
                throw new InvalidOperationException("DuiInitializer.Initialize() must be called first");

            _hwnd = NativeBridge.DuiBridge_CreateWindow(parentHwnd, title, x, y, width, height, style);

            if (_hwnd == IntPtr.Zero)
                throw new InvalidOperationException("Failed to create DirectUI window");

            _clickDelegate = OnNativeClick;
            _propDelegate = OnNativePropChange;
            _selDelegate = OnNativeSelection;
            _inputDelegate = OnNativeInput;

            NativeBridge.DuiBridge_SetClickCallback(_hwnd, _clickDelegate, IntPtr.Zero);
            NativeBridge.DuiBridge_SetPropertyCallback(_hwnd, _propDelegate, IntPtr.Zero);
            NativeBridge.DuiBridge_SetSelectionCallback(_hwnd, _selDelegate, IntPtr.Zero);
            NativeBridge.DuiBridge_SetInputCallback(_hwnd, _inputDelegate, IntPtr.Zero);

            _syncContext = SynchronizationContext.Current;
        }

        public bool LoadXml(string xmlContent, string resId = null) => NativeBridge.DuiBridge_LoadXml(_hwnd, xmlContent, resId);
        public bool LoadXmlFile(string filePath, string resId = null) => NativeBridge.DuiBridge_LoadXmlFile(_hwnd, filePath, resId);

        public bool MountFragment(string parentId, string xmlContent, string resId = null) => NativeBridge.DuiBridge_MountFragment(_hwnd, parentId, xmlContent, resId);

        public bool MountFragmentFile(string parentId, string filePath, string resId = null) => NativeBridge.DuiBridge_MountFragmentFile(_hwnd, parentId, filePath, resId);

        public bool RemoveElement(string elementId) => NativeBridge.DuiBridge_RemoveElement(_hwnd, elementId);

        public void Show() => NativeBridge.DuiBridge_ShowWindow(_hwnd, NativeConstants.SW_SHOW);
        public void Hide() => NativeBridge.DuiBridge_ShowWindow(_hwnd, NativeConstants.SW_HIDE);
        public void Maximize() => NativeBridge.DuiBridge_ShowWindow(_hwnd, NativeConstants.SW_MAXIMIZE);
        public void Minimize() => NativeBridge.DuiBridge_ShowWindow(_hwnd, NativeConstants.SW_MINIMIZE);
        public void Restore() => NativeBridge.DuiBridge_ShowWindow(_hwnd, NativeConstants.SW_RESTORE);

        public bool SetText(string elementId, string text) => NativeBridge.DuiBridge_SetText(_hwnd, elementId, text);

        public string GetText(string elementId) => MarshalAndFreeString(NativeBridge.DuiBridge_GetText(_hwnd, elementId));

        public bool SetEnabled(string elementId, bool enabled) => NativeBridge.DuiBridge_SetEnabled(_hwnd, elementId, enabled);
        public bool GetEnabled(string elementId) => NativeBridge.DuiBridge_GetEnabled(_hwnd, elementId);
        public bool SetVisible(string elementId, bool visible) => NativeBridge.DuiBridge_SetVisible(_hwnd, elementId, visible);
        public bool GetVisible(string elementId) => NativeBridge.DuiBridge_GetVisible(_hwnd, elementId);
        public bool SetAlpha(string elementId, int alpha) => NativeBridge.DuiBridge_SetAlpha(_hwnd, elementId, alpha);
        public int GetAlpha(string elementId) => NativeBridge.DuiBridge_GetAlpha(_hwnd, elementId);
        public bool SetClass(string elementId, string className) => NativeBridge.DuiBridge_SetClass(_hwnd, elementId, className);
        public string GetClass(string elementId) => MarshalAndFreeString(NativeBridge.DuiBridge_GetClass(_hwnd, elementId));
        public bool SetWidth(string elementId, int width) => NativeBridge.DuiBridge_SetWidth(_hwnd, elementId, width);
        public int GetWidth(string elementId) => NativeBridge.DuiBridge_GetWidth(_hwnd, elementId);
        public bool SetHeight(string elementId, int height) => NativeBridge.DuiBridge_SetHeight(_hwnd, elementId, height);
        public int GetHeight(string elementId) => NativeBridge.DuiBridge_GetHeight(_hwnd, elementId);
        public bool SetBackground(string elementId, Color color) => NativeBridge.DuiBridge_SetBackground(_hwnd, elementId, ToColorRef(color));
        public Color GetBackground(string elementId) => FromColorRef(NativeBridge.DuiBridge_GetBackground(_hwnd, elementId));
        public bool SetForeground(string elementId, Color color) => NativeBridge.DuiBridge_SetForeground(_hwnd, elementId, ToColorRef(color));
        public Color GetForeground(string elementId) => FromColorRef(NativeBridge.DuiBridge_GetForeground(_hwnd, elementId));

        public bool SetLayoutPos(string elementId, int layoutPos) => NativeBridge.DuiBridge_SetLayoutPos(_hwnd, elementId, layoutPos);
        public int GetLayoutPos(string elementId) => NativeBridge.DuiBridge_GetLayoutPos(_hwnd, elementId);
        public bool ElementExists(string elementId) => NativeBridge.DuiBridge_ElementExists(_hwnd, elementId);

        public bool SetProperty(string elementId, string propertyName, string value) => NativeBridge.DuiBridge_SetStringProperty(_hwnd, elementId, propertyName, value);

        public string GetStringProperty(string elementId, string propertyName) => MarshalAndFreeString(NativeBridge.DuiBridge_GetStringProperty(_hwnd, elementId, propertyName));

        public bool SetProperty(string elementId, string propertyName, int value) => NativeBridge.DuiBridge_SetIntProperty(_hwnd, elementId, propertyName, value);

        public int GetIntProperty(string elementId, string propertyName) => NativeBridge.DuiBridge_GetIntProperty(_hwnd, elementId, propertyName);

        public bool SetProperty(string elementId, string propertyName, bool value) => NativeBridge.DuiBridge_SetBoolProperty(_hwnd, elementId, propertyName, value);

        public bool GetBoolProperty(string elementId, string propertyName) => NativeBridge.DuiBridge_GetBoolProperty(_hwnd, elementId, propertyName);

        public bool SetProperty(string elementId, string propertyName, Color value) => NativeBridge.DuiBridge_SetColorProperty(_hwnd, elementId, propertyName, ToColorRef(value));

        public Color GetColorProperty(string elementId, string propertyName) => FromColorRef(NativeBridge.DuiBridge_GetColorProperty(_hwnd, elementId, propertyName));

        public bool SetSelection(string selectorId, int index) => NativeBridge.DuiBridge_SetSelection(_hwnd, selectorId, index);
        public int GetSelection(string selectorId) => NativeBridge.DuiBridge_GetSelection(_hwnd, selectorId);

        public bool SetSheet(string elementId, string sheetResId) => NativeBridge.DuiBridge_SetSheet(_hwnd, elementId, sheetResId);

        public bool SetContentBitmap(string elementId, IntPtr hBitmap) => NativeBridge.DuiBridge_SetContentBitmap(_hwnd, elementId, hBitmap);

        public DuiElement Element(string elementId) => new DuiElement(this, elementId);

        public bool StartDefer() => NativeBridge.DuiBridge_StartDefer(_hwnd);
        public bool EndDefer() => NativeBridge.DuiBridge_EndDefer(_hwnd);

        public IDisposable BeginDefer() => new DeferScope(this);

        public static void RunMessagePump() => NativeBridge.DuiBridge_RunMessagePump();
        public static void StopMessagePump() => NativeBridge.DuiBridge_StopMessagePump();

        public static void SetParseErrorCallback(Action<string, string, int> handler)
        {
            if (handler == null)
            {
                NativeBridge.DuiBridge_SetParseErrorCallback(null, IntPtr.Zero);
                return;
            }

            var cb = new NativeBridge.ParseErrorCallback(
                (err, tok, line, _) => handler(err, tok, line));
            s_parseErrorDelegate = cb;
            NativeBridge.DuiBridge_SetParseErrorCallback(cb, IntPtr.Zero);
        }

        private static NativeBridge.ParseErrorCallback s_parseErrorDelegate;

        private static uint ToColorRef(Color c) => (uint)(c.R | (c.G << 8) | (c.B << 16));
        private static Color FromColorRef(uint cr) => Color.FromArgb((int)(cr & 0xFF), (int)((cr >> 8) & 0xFF), (int)((cr >> 16) & 0xFF));

        private static string MarshalAndFreeString(IntPtr ptr)
        {
            if (ptr == IntPtr.Zero) return null;

            string result = Marshal.PtrToStringUni(ptr);
            NativeBridge.DuiBridge_FreeString(ptr);

            return result;
        }

        private void FireEvent<T>(EventHandler<T> handler, T args) where T : EventArgs
        {
            if (handler == null) return;
            if (_syncContext != null)
                _syncContext.Post(_ => handler(this, args), null);
            else
                handler(this, args);
        }

        private void OnNativeClick(string elementId, IntPtr context)
        {
            long now = Stopwatch.GetTimestamp();

            lock (_clickLock)
            {
                if (now - _lastClickTick < ClickDebounceThreshold)
                    return;

                _lastClickTick = now;
            }

            FireEvent(ButtonClicked, new DuiClickEventArgs(elementId));
        }

        private void OnNativePropChange(string elementId, string propertyName, IntPtr context) => FireEvent(PropertyChanged, new DuiPropertyEventArgs(elementId, propertyName));

        private void OnNativeSelection(string selectorId, string selectedId, int selectedIndex, IntPtr context) => FireEvent(SelectionChanged, new DuiSelectionEventArgs(selectorId, selectedId, selectedIndex));

        private void OnNativeInput(string elementId, int device, int code, int flags, IntPtr context) => FireEvent(Input, new DuiInputEventArgs(elementId, device, code, flags));

        public void Dispose()
        {
            if (_disposed) return;
            _disposed = true;

            if (_hwnd != IntPtr.Zero)
            {
                NativeBridge.DuiBridge_DestroyWindow(_hwnd);
                _hwnd = IntPtr.Zero;
            }

            GC.SuppressFinalize(this);
        }

        ~DuiWindow() { Dispose(); }

        private sealed class DeferScope : IDisposable
        {
            private DuiWindow _w;
            public DeferScope(DuiWindow w) { _w = w; w.StartDefer(); }
            public void Dispose() { if (_w != null) { _w.EndDefer(); _w = null; } }
        }
    }
}