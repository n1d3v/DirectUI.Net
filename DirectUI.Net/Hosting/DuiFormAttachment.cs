using System;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using DirectUI.Net.Core;
using DirectUI.Net.Events;
using DirectUI.Net.Interop;

namespace DirectUI.Net.Hosting
{
    public sealed class DuiFormAttachment : IDisposable
    {
        private readonly Form _form;
        private DuiWindow _duiWindow;
        private bool _disposed;

        public DuiWindow DuiWindow => _duiWindow;

        // All events available in DirectUI.Net
        public event EventHandler<DuiClickEventArgs> ButtonClicked;
        public event EventHandler<DuiPropertyEventArgs> PropertyChanged;
        public event EventHandler<DuiSelectionEventArgs> SelectionChanged;
        public event EventHandler<DuiInputEventArgs> Input;

        public DuiFormAttachment(Form form)
        {
            _form = form ?? throw new ArgumentNullException(nameof(form));

            if (_form.IsHandleCreated)
                CreateDuiWindow();
            else
                _form.HandleCreated += OnHandleCreated;

            _form.ClientSizeChanged += OnClientSizeChanged;
            _form.FormClosed += OnFormClosed;
        }

        public bool LoadXmlFile(string filePath, string resId = null) => _duiWindow?.LoadXmlFile(filePath, resId) ?? false;

        public bool LoadXml(string xmlContent, string resId = null) => _duiWindow?.LoadXml(xmlContent, resId) ?? false;

        public bool MountFragment(string parentId, string xmlContent, string resId = null) => _duiWindow?.MountFragment(parentId, xmlContent, resId) ?? false;

        public bool MountFragmentFile(string parentId, string filePath, string resId = null) => _duiWindow?.MountFragmentFile(parentId, filePath, resId) ?? false;

        public DuiElement Element(string elementId) => _duiWindow?.Element(elementId);

        public IDisposable BeginDefer() => _duiWindow?.BeginDefer();

        private void OnHandleCreated(object sender, EventArgs e)
        {
            _form.HandleCreated -= OnHandleCreated;
            CreateDuiWindow();
        }

        private void CreateDuiWindow()
        {
            if (_duiWindow != null) return;

            _duiWindow = new DuiWindow(
                title: _form.Text,
                x: 0, y: 0,
                width: _form.ClientSize.Width,
                height: _form.ClientSize.Height,
                parentHwnd: _form.Handle,
                style: NativeConstants.WS_CHILD | NativeConstants.WS_VISIBLE | NativeConstants.WS_CLIPSIBLINGS);
            SetWindowPos(_duiWindow.Handle, new IntPtr(1), 0, 0, 0, 0, 0x0003);

            _duiWindow.ButtonClicked += (s, args) => ButtonClicked?.Invoke(this, args);
            _duiWindow.PropertyChanged += (s, args) => PropertyChanged?.Invoke(this, args);
            _duiWindow.SelectionChanged += (s, args) => SelectionChanged?.Invoke(this, args);
            _duiWindow.Input += (s, args) => Input?.Invoke(this, args);
        }

        private void OnClientSizeChanged(object sender, EventArgs e)
        {
            if (_duiWindow == null || _duiWindow.Handle == IntPtr.Zero) return;
            SetWindowPos(_duiWindow.Handle, IntPtr.Zero, 0, 0, _form.ClientSize.Width, _form.ClientSize.Height, 0x0004 | 0x0002);
        }

        private void OnFormClosed(object sender, FormClosedEventArgs e) => Dispose();

        public void Dispose()
        {
            if (_disposed) return;
            _disposed = true;

            _form.HandleCreated -= OnHandleCreated;
            _form.ClientSizeChanged -= OnClientSizeChanged;
            _form.FormClosed -= OnFormClosed;

            _duiWindow?.Dispose();
            _duiWindow = null;
        }

        [DllImport("user32.dll")]
        private static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter,
            int x, int y, int cx, int cy, uint flags);
    }
}