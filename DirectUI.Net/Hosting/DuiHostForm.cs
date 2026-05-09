using System;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using DirectUI.Net.Core;
using DirectUI.Net.Events;
using DirectUI.Net.Interop;

namespace DirectUI.Net.Hosting
{
    public class DuiHostForm : Form
    {
        private DuiWindow _duiWindow;
        public DuiWindow DuiWindow => _duiWindow;

        // All events available in DirectUI.Net
        public event EventHandler<DuiClickEventArgs> DuiButtonClicked;
        public event EventHandler<DuiPropertyEventArgs> DuiPropertyChanged;
        public event EventHandler<DuiSelectionEventArgs> DuiSelectionChanged;
        public event EventHandler<DuiInputEventArgs> DuiInput;

        public DuiHostForm() { SetStyle(ControlStyles.AllPaintingInWmPaint | ControlStyles.UserPaint, true); }

        protected override void OnHandleCreated(EventArgs e)
        {
            base.OnHandleCreated(e);
            CreateDuiWindow();
        }

        private void CreateDuiWindow()
        {
            if (_duiWindow != null) return;

            _duiWindow = new DuiWindow(
                Text, 0, 0, ClientSize.Width, ClientSize.Height,
                Handle, NativeConstants.WS_CHILD | NativeConstants.WS_VISIBLE);

            _duiWindow.ButtonClicked += (s, args) => DuiButtonClicked?.Invoke(this, args);
            _duiWindow.PropertyChanged += (s, args) => DuiPropertyChanged?.Invoke(this, args);
            _duiWindow.SelectionChanged += (s, args) => DuiSelectionChanged?.Invoke(this, args);
            _duiWindow.Input += (s, args) => DuiInput?.Invoke(this, args);
        }

        public bool LoadXml(string xmlContent, string resId = null)
        {
            if (_duiWindow == null) return false;
            return _duiWindow.LoadXml(xmlContent, resId);
        }

        public bool LoadXmlFile(string filePath, string resId = null)
        {
            if (_duiWindow == null) return false;
            return _duiWindow.LoadXmlFile(filePath, resId);
        }

        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);
            if (_duiWindow != null && _duiWindow.Handle != IntPtr.Zero) { SetWindowPos(_duiWindow.Handle, IntPtr.Zero, 0, 0, ClientSize.Width, ClientSize.Height, 0x0004 | 0x0002); }
        }

        protected override void OnFormClosing(FormClosingEventArgs e)
        {
            base.OnFormClosing(e);
            _duiWindow?.Dispose();
            _duiWindow = null;
        }

        [DllImport("user32.dll")]
        private static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter,
            int x, int y, int cx, int cy, uint flags);
    }
}