using System.IO;

namespace DirectUI.Net.Core
{
    public static class DuiParser
    {
        public static string LoadFromFile(string fileName)
        {
            string path = DuiInitializer.Instance.ResolveXmlPath(fileName);
            if (!File.Exists(path)) { return null; }

            return File.ReadAllText(path);
        }

        public static bool LoadAndApply(DuiWindow window, string fileName, string resId = null)
        {
            string path = DuiInitializer.Instance.ResolveXmlPath(fileName);
            if (!File.Exists(path)) { return false; }

            return window.LoadXmlFile(path, resId);
        }
    }
}