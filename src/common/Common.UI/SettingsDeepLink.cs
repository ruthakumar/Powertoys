﻿// Copyright (c) Microsoft Corporation
// The Microsoft Corporation licenses this file to you under the MIT license.
// See the LICENSE file in the project root for more information.

using System.Diagnostics;
using System.IO;

namespace Common.UI
{
    public static class SettingsDeepLink
    {
        public enum SettingsWindow
        {
            Overview = 0,
            Awake,
            ColorPicker,
            FancyZones,
            Run,
            ImageResizer,
            KBM,
            MouseUtils,
            PowerRename,
            FileExplorer,
            ShortcutGuide,
            VideoConference,
            Hosts,
            RegistryPreview,
        }

        private static string SettingsWindowNameToString(SettingsWindow value)
        {
            switch (value)
            {
                case SettingsWindow.Overview:
                    return "Overview";
                case SettingsWindow.Awake:
                    return "Awake";
                case SettingsWindow.ColorPicker:
                    return "ColorPicker";
                case SettingsWindow.FancyZones:
                    return "FancyZones";
                case SettingsWindow.Run:
                    return "Run";
                case SettingsWindow.ImageResizer:
                    return "ImageResizer";
                case SettingsWindow.KBM:
                    return "KBM";
                case SettingsWindow.MouseUtils:
                    return "MouseUtils";
                case SettingsWindow.PowerRename:
                    return "PowerRename";
                case SettingsWindow.FileExplorer:
                    return "FileExplorer";
                case SettingsWindow.ShortcutGuide:
                    return "ShortcutGuide";
                case SettingsWindow.VideoConference:
                    return "VideoConference";
                case SettingsWindow.Hosts:
                    return "Hosts";
                case SettingsWindow.RegistryPreview:
                    return "RegistryPreview";
                default:
                    {
                        return string.Empty;
                    }
            }
        }

        public static void OpenSettings(SettingsWindow window)
        {
            try
            {
                var assemblyPath = Path.GetDirectoryName(System.Reflection.Assembly.GetExecutingAssembly().Location);
                var fullPath = Directory.GetParent(assemblyPath).FullName;
                Process.Start(new ProcessStartInfo(fullPath + "\\..\\PowerToys.exe") { Arguments = "--open-settings=" + SettingsWindowNameToString(window) });
            }
            catch
            {
                // TODO(stefan): Log exception once unified logging is implemented
            }
        }
    }
}
