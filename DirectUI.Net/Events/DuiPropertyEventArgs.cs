using System;

namespace DirectUI.Net.Events
{
    public class DuiPropertyEventArgs : EventArgs
    {
        public string ElementId { get; }
        public string PropertyName { get; }

        public DuiPropertyEventArgs(string elementId, string propertyName)
        {
            ElementId = elementId;
            PropertyName = propertyName;
        }
    }
}