using System;
using System.IO;
using System.Runtime.InteropServices;
using DirectUI.Net.Core;

namespace DirectUI.Net
{
    public sealed class DuiInitializer
    {
        private static readonly Lazy<DuiInitializer> _instance = new Lazy<DuiInitializer>(() => new DuiInitializer());

        public static DuiInitializer Instance => _instance.Value;

        public string XmlDirectory { get; set; }
        public string AssetDirectory { get; set; }
        public bool Initialized { get; private set; }

        private DuiInitializer() { }

        private static void LoadSystemDll(string name)
        {
            if (NativeLibraryLoader.LoadLibraryExW(name, IntPtr.Zero, NativeLibraryLoader.LOAD_LIBRARY_SEARCH_SYSTEM32) == IntPtr.Zero)
            {
                int err = Marshal.GetLastWin32Error();
                throw new DllNotFoundException($"Failed to load system '{name}' (Win32 error {err})");
            }
        }

        private static void LoadNative(string path)
        {
            if (NativeLibraryLoader.LoadLibrary(path) == IntPtr.Zero)
            {
                int err = Marshal.GetLastWin32Error();
                throw new DllNotFoundException($"Failed to load '{Path.GetFileName(path)}' (Win32 error {err}): {path}");
            }
        }

        public bool Initialize()
        {
            if (Initialized) return true;

            LoadSystemDll("duser.dll");
            LoadSystemDll("dui70.dll");

            string arch = IntPtr.Size == 8 ? "x64" : "Win32";
            string basePath = Path.GetDirectoryName(typeof(DuiInitializer).Assembly.Location);
            string bridgePath = Path.Combine(basePath, "Libraries", arch, "dui_bridge.dll");

            if (!File.Exists(bridgePath))
                throw new DllNotFoundException($"dui_bridge.dll not found at: {bridgePath}");

            LoadNative(bridgePath);

            IntPtr hInst = NativeLibraryLoader.GetModuleHandle(null);
            bool result = Interop.NativeBridge.DuiBridge_Init(hInst);

            if (result)
                Initialized = true;

            return result;
        }

        public void Shutdown()
        {
            if (!Initialized) return;
            Interop.NativeBridge.DuiBridge_Shutdown();
            Initialized = false;
        }

        public void SetParseErrorHandler(Action<string, string, int> handler) => DuiWindow.SetParseErrorCallback(handler);

        public string ResolveXmlPath(string fileName)
        {
            if (string.IsNullOrEmpty(XmlDirectory)) return fileName;
            return Path.Combine(XmlDirectory, fileName);
        }

        public string ResolveAssetPath(string fileName)
        {
            if (string.IsNullOrEmpty(AssetDirectory)) return fileName;
            return Path.Combine(AssetDirectory, fileName);
        }
    }

    internal static class NativeLibraryLoader
    {
        public const uint LOAD_LIBRARY_SEARCH_SYSTEM32 = 0x00000800;

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true, EntryPoint = "LoadLibraryW")]
        public static extern IntPtr LoadLibrary(string lpFileName);

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        public static extern IntPtr LoadLibraryExW(string lpFileName, IntPtr hFile, uint dwFlags);

        [DllImport("kernel32.dll", CharSet = CharSet.Unicode, SetLastError = true)]
        public static extern IntPtr GetModuleHandle(string lpModuleName);
    }
}