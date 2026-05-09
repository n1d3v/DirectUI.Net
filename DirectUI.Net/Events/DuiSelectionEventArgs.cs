using System;

namespace DirectUI.Net.Events
{
    public class DuiSelectionEventArgs : EventArgs
    {
        public string SelectorId { get; }
        public string SelectedId { get; }
        public int SelectedIndex { get; }

        public DuiSelectionEventArgs(string selectorId, string selectedId, int selectedIndex)
        {
            SelectorId = selectorId;
            SelectedId = selectedId;
            SelectedIndex = selectedIndex;
        }
    }
}