using System;
using System.ComponentModel;
using System.Drawing;
using System.Runtime.InteropServices;
using System.Windows.Forms;
using DirectUI.Net.Core;
using DirectUI.Net.Events;
using DirectUI.Net.Interop;

namespace DirectUI.Net.Hosting
{
    [ToolboxItem(true)]
    [Description("Renders a DirectUI layout inside a WinForms control area.")]
    [DefaultEvent("ButtonClicked")]
    [DefaultProperty("XmlFile")]
    public class DirectUIRenderer : UserControl
    {
        private string _xmlFile;
        private string _resId = "main";

        [Category("DirectUI")]
        [Description("DUIXML file to load automatically. Relative to DuiInitializer.XmlDirectory.")]
        [Browsable(true)]
        [DefaultValue(null)]
        public string XmlFile
        {
            get => _xmlFile;
            set
            {
                _xmlFile = value;
                if (!DesignMode && _duiWindow != null && !string.IsNullOrEmpty(value))
                    LoadXmlFile(value, _resId);
            }
        }

        [Category("DirectUI")]
        [Description("The resid of the root DUIXML element. Defaults to \"main\".")]
        [Browsable(true)]
        [DefaultValue("main")]
        public string ResId
        {
            get => _resId;
            set => _resId = string.IsNullOrEmpty(value) ? "main" : value;
        }

        private DuiWindow _duiWindow;

        [Browsable(false)]
        public DuiWindow DuiWindow => _duiWindow;

        // All events available in DirectUI.Net
        [Category("DirectUI")]
        [Description("Raised when a DirectUI button element is clicked.")]
        public event EventHandler<DuiClickEventArgs> ButtonClicked;

        [Category("DirectUI")]
        [Description("Raised when a DirectUI element property changes.")]
        public event EventHandler<DuiPropertyEventArgs> PropertyChanged;

        [Category("DirectUI")]
        [Description("Raised when a DirectUI Selector's selection changes.")]
        public event EventHandler<DuiSelectionEventArgs> SelectionChanged;

        [Category("DirectUI")]
        [Description("Raised for input events (mouse, keyboard, touch) inside the DirectUI tree.")]
        public event EventHandler<DuiInputEventArgs> Input;

        public DirectUIRenderer()
        {
            SetStyle(
                ControlStyles.AllPaintingInWmPaint |
                ControlStyles.UserPaint |
                ControlStyles.Opaque,
                true);

            BackColor = SystemColors.AppWorkspace;
        }

        protected override void OnHandleCreated(EventArgs e)
        {
            base.OnHandleCreated(e);

            if (DesignMode) return;

            if (!DuiInitializer.Instance.Initialized)
                return;

            _duiWindow = new DuiWindow(
                title: Name ?? "DirectUIRenderer",
                x: 0, y: 0,
                width: Width,
                height: Height,
                parentHwnd: Handle,
                style: NativeConstants.WS_CHILD | NativeConstants.WS_VISIBLE);

            _duiWindow.ButtonClicked += (s, args) => ButtonClicked?.Invoke(this, args);
            _duiWindow.PropertyChanged += (s, args) => PropertyChanged?.Invoke(this, args);
            _duiWindow.SelectionChanged += (s, args) => SelectionChanged?.Invoke(this, args);
            _duiWindow.Input += (s, args) => Input?.Invoke(this, args);

            if (!string.IsNullOrEmpty(_xmlFile))
                LoadXmlFile(_xmlFile, _resId);
        }

        protected override void OnPaint(PaintEventArgs e)
        {
            if (!DesignMode) return;

            var g = e.Graphics;
            g.Clear(Color.FromArgb(30, 30, 30));

            float pw = 2f;
            using var borderPen = new Pen(Color.FromArgb(0, 120, 215), pw);

            float half = pw / 2f;
            g.DrawRectangle(borderPen, half, half, Width - pw, Height - pw);

            const string label = "DirectUIRenderer";

            using var font = new Font("Segoe UI", 12f, FontStyle.Regular, GraphicsUnit.Point);
            var textSize = g.MeasureString(label, font);
            var textX = (Width - textSize.Width) / 2f;
            var textY = (Height - textSize.Height) / 2f;

            using var brush = new SolidBrush(Color.FromArgb(0, 120, 215));
            g.DrawString(label, font, brush, textX, textY);

            if (!string.IsNullOrEmpty(_xmlFile))
            {
                var sub = _xmlFile;
                using var subFont = new Font("Segoe UI", 9f, FontStyle.Regular, GraphicsUnit.Point);

                var subSize = g.MeasureString(sub, subFont);
                using var subBrush = new SolidBrush(Color.FromArgb(160, 160, 160));

                g.DrawString(sub, subFont, subBrush,
                    (Width - subSize.Width) / 2f, textY + textSize.Height + 2f);
            }
        }

        protected override void OnResize(EventArgs e)
        {
            base.OnResize(e);

            if (_duiWindow == null || _duiWindow.Handle == IntPtr.Zero) return;
            SetWindowPos(_duiWindow.Handle, IntPtr.Zero, 0, 0, Width, Height, 0x0004 | 0x0002);
        }

        public bool LoadXmlFile(string filePath, string resId = null)
        {
            if (_duiWindow == null) return false;
            string path = DuiInitializer.Instance.ResolveXmlPath(filePath);
            return _duiWindow.LoadXmlFile(path, resId ?? _resId);
        }

        public bool LoadXml(string xmlContent, string resId = null) => _duiWindow?.LoadXml(xmlContent, resId ?? _resId) ?? false;

        public bool MountFragment(string parentId, string xmlContent, string resId = null) => _duiWindow?.MountFragment(parentId, xmlContent, resId) ?? false;

        public bool MountFragmentFile(string parentId, string filePath, string resId = null) => _duiWindow?.MountFragmentFile(parentId, filePath, resId) ?? false;

        public DuiElement Element(string elementId) => _duiWindow?.Element(elementId);

        public bool SetCueBanner(string elementId, string text) => _duiWindow?.SetCueBanner(elementId, text) ?? false;
        public bool SetCueBannerItalic(string elementId) => _duiWindow?.SetCueBannerItalic(elementId) ?? false;

        public IDisposable BeginDefer() => _duiWindow?.BeginDefer();

        protected override void Dispose(bool disposing)
        {
            if (disposing)
            {
                _duiWindow?.Dispose();
                _duiWindow = null;
            }
            base.Dispose(disposing);
        }

        [DllImport("user32.dll")]
        private static extern bool SetWindowPos(IntPtr hWnd, IntPtr hWndInsertAfter,
            int x, int y, int cx, int cy, uint flags);
    }
}