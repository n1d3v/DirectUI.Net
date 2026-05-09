using System;
using System.IO;

namespace DirectUI.Net.Core
{
    public abstract class DuiUserControl : IDisposable
    {
        private DuiWindow _host;
        private string _parentId;
        private bool _mounted;
        private bool _disposed;

        public DuiWindow Host => _host;
        public string ParentId => _parentId;
        public bool IsMounted => _mounted;

        protected abstract string ResId { get; }
        protected virtual string XmlSource => null;
        protected virtual string XmlFileName => null;

        public bool Mount(DuiWindow window, string parentId)
        {
            if (window == null) throw new ArgumentNullException(nameof(window));
            if (string.IsNullOrEmpty(parentId)) throw new ArgumentException("Parent ID required", nameof(parentId));
            if (_mounted) return false;

            _host = window;
            _parentId = parentId;

            bool ok;
            if (!string.IsNullOrEmpty(XmlSource))
            {
                ok = window.MountFragment(parentId, XmlSource, ResId);
            }
            else if (!string.IsNullOrEmpty(XmlFileName))
            {
                string path = ResolvePath(XmlFileName);
                ok = window.MountFragmentFile(parentId, path, ResId);
            }
            else
            {
                throw new InvalidOperationException(
                    $"{GetType().Name} must override XmlSource or XmlFileName.");
            }

            if (ok)
            {
                _mounted = true;
                window.ButtonClicked += DispatchClick;
                window.PropertyChanged += DispatchPropertyChanged;
                window.SelectionChanged += DispatchSelection;
                window.Input += DispatchInput;

                OnMounted();
            }
            return ok;
        }

        public bool Unmount()
        {
            if (!_mounted || _host == null) return false;

            _host.ButtonClicked -= DispatchClick;
            _host.PropertyChanged -= DispatchPropertyChanged;
            _host.SelectionChanged -= DispatchSelection;
            _host.Input -= DispatchInput;

            bool ok = _host.RemoveElement(ResId);

            _mounted = false;
            OnUnmounted();

            _host = null;
            _parentId = null;
            return ok;
        }

        public DuiElement Element(string elementId) => _host?.Element(elementId);
        public DuiElement Root => _host?.Element(ResId);

        protected virtual void OnMounted() { }
        protected virtual void OnUnmounted() { }
        protected virtual void OnButtonClicked(string elementId) { }
        protected virtual void OnPropertyChanged(string elementId, string propertyName) { }
        protected virtual void OnSelectionChanged(string selectorId, string selectedId, int selectedIndex) { }
        protected virtual void OnInput(string elementId, int device, int code, int flags) { }

        public void Dispose()
        {
            Dispose(true);
            GC.SuppressFinalize(this);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (_disposed) return;
            _disposed = true;
            if (disposing && _mounted)
                Unmount();
        }

        ~DuiUserControl() { Dispose(false); }

        private void DispatchClick(object sender, Events.DuiClickEventArgs e) => OnButtonClicked(e.ElementId);

        private void DispatchPropertyChanged(object sender, Events.DuiPropertyEventArgs e) => OnPropertyChanged(e.ElementId, e.PropertyName);

        private void DispatchSelection(object sender, Events.DuiSelectionEventArgs e) => OnSelectionChanged(e.SelectorId, e.SelectedId, e.SelectedIndex);

        private void DispatchInput(object sender, Events.DuiInputEventArgs e) => OnInput(e.ElementId, (int)e.Device, e.Code, e.Flags);

        private static string ResolvePath(string fileName)
        {
            if (Path.IsPathRooted(fileName)) return fileName;
            return DuiInitializer.Instance.ResolveXmlPath(fileName);
        }
    }
}