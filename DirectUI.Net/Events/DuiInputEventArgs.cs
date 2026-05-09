using System;

namespace DirectUI.Net.Events
{
    public enum DuiInputDevice
    {
        Mouse = 1,
        Keyboard = 2,
        Stylus = 3,
        Touch = 4,
    }

    public class DuiInputEventArgs : EventArgs
    {
        public string ElementId { get; }
        public DuiInputDevice Device { get; }
        public int Code { get; }
        public int Flags { get; }

        public DuiInputEventArgs(string elementId, int device, int code, int flags)
        {
            ElementId = elementId;
            Device = (DuiInputDevice)device;
            Code = code;
            Flags = flags;
        }
    }
}