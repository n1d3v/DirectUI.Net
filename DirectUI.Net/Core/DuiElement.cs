using System;
using System.Drawing;

namespace DirectUI.Net.Core
{
    public class DuiElement
    {
        private readonly DuiWindow _window;
        private readonly string _id;

        public string Id => _id;
        public bool Exists => _window.ElementExists(_id);
        public DuiWindow Window => _window;

        public DuiElement(DuiWindow window, string elementId)
        {
            _window = window ?? throw new ArgumentNullException(nameof(window));
            _id = elementId ?? throw new ArgumentNullException(nameof(elementId));
        }

        public DuiElement SetText(string text) { _window.SetText(_id, text); return this; }
        public string GetText() => _window.GetText(_id);

        public DuiElement SetEnabled(bool enabled) { _window.SetEnabled(_id, enabled); return this; }
        public bool GetEnabled() => _window.GetEnabled(_id);

        public DuiElement SetVisible(bool visible) { _window.SetVisible(_id, visible); return this; }
        public bool GetVisible() => _window.GetVisible(_id);

        public DuiElement SetAlpha(int alpha) { _window.SetAlpha(_id, alpha); return this; }
        public int GetAlpha() => _window.GetAlpha(_id);

        public DuiElement SetClass(string className) { _window.SetClass(_id, className); return this; }
        public string GetClass() => _window.GetClass(_id);

        public DuiElement SetSheet(string sheetResId) { _window.SetSheet(_id, sheetResId); return this; }

        public DuiElement SetBackground(Color color) { _window.SetBackground(_id, color); return this; }
        public Color GetBackground() => _window.GetBackground(_id);

        public DuiElement SetForeground(Color color) { _window.SetForeground(_id, color); return this; }
        public Color GetForeground() => _window.GetForeground(_id);

        public DuiElement SetWidth(int width) { _window.SetWidth(_id, width); return this; }
        public int GetWidth() => _window.GetWidth(_id);

        public DuiElement SetHeight(int height) { _window.SetHeight(_id, height); return this; }
        public int GetHeight() => _window.GetHeight(_id);

        public DuiElement SetSize(int width, int height)
        {
            _window.SetWidth(_id, width);
            _window.SetHeight(_id, height);
            return this;
        }

        public DuiElement SetLayoutPos(int layoutPos) { _window.SetLayoutPos(_id, layoutPos); return this; }
        public int GetLayoutPos() => _window.GetLayoutPos(_id);

        public DuiElement SetProperty(string propertyName, string value) { _window.SetProperty(_id, propertyName, value); return this; }
        public DuiElement SetProperty(string propertyName, int value) { _window.SetProperty(_id, propertyName, value); return this; }
        public DuiElement SetProperty(string propertyName, bool value) { _window.SetProperty(_id, propertyName, value); return this; }
        public DuiElement SetProperty(string propertyName, Color value) { _window.SetProperty(_id, propertyName, value); return this; }

        public string GetStringProperty(string propertyName) => _window.GetStringProperty(_id, propertyName);
        public int GetIntProperty(string propertyName) => _window.GetIntProperty(_id, propertyName);
        public bool GetBoolProperty(string propertyName) => _window.GetBoolProperty(_id, propertyName);
        public Color GetColorProperty(string propertyName) => _window.GetColorProperty(_id, propertyName);

        public bool Remove() => _window.RemoveElement(_id);

        public bool MountFragment(string xmlContent, string resId = null) => _window.MountFragment(_id, xmlContent, resId);
        public bool MountFragmentFile(string filePath, string resId = null) => _window.MountFragmentFile(_id, filePath, resId);

        public DuiElement SetSelection(int index) { _window.SetSelection(_id, index); return this; }
        public int GetSelection() => _window.GetSelection(_id);
    }
}