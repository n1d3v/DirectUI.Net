using System;

namespace DirectUI.Net.Events
{
    public class DuiClickEventArgs : EventArgs
    {
        public string ElementId { get; }
        public DuiClickEventArgs(string elementId) { ElementId = elementId; }
    }
}