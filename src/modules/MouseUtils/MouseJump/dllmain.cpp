#include "pch.h"

#include <interface/powertoy_module_interface.h>
//#include <interface/lowlevel_keyboard_event_data.h>
//#include <interface/win_hook_event_data.h>
#include <common/SettingsAPI/settings_objects.h>
#include "trace.h"
#include <common/utils/winapi_error.h>
#include <common/utils/logger_helper.h>

extern "C" IMAGE_DOS_HEADER __ImageBase;

HMODULE m_hModule;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
    m_hModule = hModule;
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        Trace::RegisterProvider();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        Trace::UnregisterProvider();
        break;
    }
    return TRUE;
}

// The PowerToy name that will be shown in the settings.
const static wchar_t* MODULE_NAME = L"MouseJump";
// Add a description that will we shown in the module settings page.
const static wchar_t* MODULE_DESC = L"Quickly move the mouse long distances";

namespace
{
    const wchar_t JSON_KEY_PROPERTIES[] = L"properties";
    const wchar_t JSON_KEY_WIN[] = L"win";
    const wchar_t JSON_KEY_ALT[] = L"alt";
    const wchar_t JSON_KEY_CTRL[] = L"ctrl";
    const wchar_t JSON_KEY_SHIFT[] = L"shift";
    const wchar_t JSON_KEY_CODE[] = L"code";
    const wchar_t JSON_KEY_ACTIVATION_SHORTCUT[] = L"activation_shortcut";
}

// Implement the PowerToy Module Interface and all the required methods.
class MouseJump : public PowertoyModuleIface
{
private:
    // The PowerToy state.
    bool m_enabled = false;

    // Hotkey to invoke the module
    Hotkey m_hotkey;
    HANDLE m_hProcess;

    void parse_hotkey(PowerToysSettings::PowerToyValues& settings)
    {
        auto settingsObject = settings.get_raw_json();
        if (settingsObject.GetView().Size())
        {
            try
            {
                auto jsonHotkeyObject = settingsObject.GetNamedObject(JSON_KEY_PROPERTIES).GetNamedObject(JSON_KEY_ACTIVATION_SHORTCUT);
                m_hotkey.win = jsonHotkeyObject.GetNamedBoolean(JSON_KEY_WIN);
                m_hotkey.alt = jsonHotkeyObject.GetNamedBoolean(JSON_KEY_ALT);
                m_hotkey.shift = jsonHotkeyObject.GetNamedBoolean(JSON_KEY_SHIFT);
                m_hotkey.ctrl = jsonHotkeyObject.GetNamedBoolean(JSON_KEY_CTRL);
                m_hotkey.key = static_cast<unsigned char>(jsonHotkeyObject.GetNamedNumber(JSON_KEY_CODE));
            }
            catch (...)
            {
                Logger::error("Failed to initialize Mouse Jump start shortcut");
            }
        }
        else
        {
            Logger::info("MouseJump settings are empty");
        }

        if (!m_hotkey.key)
        {
            Logger::info("MouseJump is going to use default shortcut");
            m_hotkey.win = true;
            m_hotkey.alt = false;
            m_hotkey.shift = true;
            m_hotkey.ctrl = false;
            m_hotkey.key = 'D';
        }
    }

    bool is_process_running()
    {
        return WaitForSingleObject(m_hProcess, 0) == WAIT_TIMEOUT;
    }

    void launch_process()
    {
        Logger::trace(L"Starting MouseJump process");
        unsigned long powertoys_pid = GetCurrentProcessId();

        std::wstring executable_args = L"";
        executable_args.append(std::to_wstring(powertoys_pid));

        SHELLEXECUTEINFOW sei{ sizeof(sei) };
        sei.fMask = { SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI };
        sei.lpFile = L"modules\\MouseUtils\\MouseJumpUI\\PowerToys.MouseJumpUI.exe";
        sei.nShow = SW_SHOWNORMAL;
        sei.lpParameters = executable_args.data();
        if (ShellExecuteExW(&sei))
        {
            Logger::trace("Successfully started the Mouse Jump process");
        }
        else
        {
            Logger::error(L"Mouse Jump failed to start. {}", get_last_error_or_default(GetLastError()));
        }

        m_hProcess = sei.hProcess;
    }

    // Load initial settings from the persisted values.
    void init_settings();

    void terminate_process()
    {
        TerminateProcess(m_hProcess, 1);
    }

public:
    // Constructor
    MouseJump()
    {
        LoggerHelpers::init_logger(MODULE_NAME, L"ModuleInterface", LogSettings::mouseJumpLoggerName);
        init_settings();
    };

    ~MouseJump()
    {
        if (m_enabled)
        {
            terminate_process();
        }
        m_enabled = false;
    }

    // Destroy the powertoy and free memory
    virtual void destroy() override
    {
        delete this;
    }

    // Return the display name of the powertoy, this will be cached by the runner
    virtual const wchar_t* get_name() override
    {
        return MODULE_NAME;
    }

    // Return the non localized key of the powertoy, this will be cached by the runner
    virtual const wchar_t* get_key() override
    {
        return MODULE_NAME;
    }

    // Return the configured status for the gpo policy for the module
    virtual powertoys_gpo::gpo_rule_configured_t gpo_policy_enabled_configuration() override
    {
        return powertoys_gpo::getConfiguredMouseJumpEnabledValue();
    }

    // Return JSON with the configuration options.
    virtual bool get_config(wchar_t* buffer, int* buffer_size) override
    {
        HINSTANCE hinstance = reinterpret_cast<HINSTANCE>(&__ImageBase);

        // Create a Settings object.
        PowerToysSettings::Settings settings(hinstance, get_name());
        settings.set_description(MODULE_DESC);

        return settings.serialize_to_buffer(buffer, buffer_size);
    }

    // Signal from the Settings editor to call a custom action.
    // This can be used to spawn more complex editors.
    virtual void call_custom_action(const wchar_t* action) override
    {
        static UINT custom_action_num_calls = 0;
        try
        {
            // Parse the action values, including name.
            PowerToysSettings::CustomActionObject action_object =
                PowerToysSettings::CustomActionObject::from_json_string(action);

            //if (action_object.get_name() == L"custom_action_id") {
            //  // Execute your custom action
            //}
        }
        catch (std::exception&)
        {
            // Improper JSON.
        }
    }

    // Called by the runner to pass the updated settings values as a serialized JSON.
    virtual void set_config(const wchar_t* config) override
    {
        try
        {
            // Parse the input JSON string.
            PowerToysSettings::PowerToyValues values =
                PowerToysSettings::PowerToyValues::from_json_string(config, get_key());

            parse_hotkey(values);

            values.save_to_settings_file();
        }
        catch (std::exception&)
        {
            // Improper JSON.
        }
    }

    // Enable the powertoy
    virtual void enable()
    {
        m_enabled = true;
        Trace::EnableJumpTool(true);
    }

    // Disable the powertoy
    virtual void disable()
    {
        if (m_enabled)
        {
            terminate_process();
        }

        m_enabled = false;
        Trace::EnableJumpTool(false);
    }

    // Returns if the powertoys is enabled
    virtual bool is_enabled() override
    {
        return m_enabled;
    }

    virtual bool on_hotkey(size_t /*hotkeyId*/) override
    {
        if (m_enabled)
        {
            Logger::trace(L"MouseJump hotkey pressed");
            Trace::InvokeJumpTool();
            if (is_process_running())
            {
                terminate_process();
            }
            launch_process();

            return true;
        }

        return false;
    }

    virtual size_t get_hotkeys(Hotkey* hotkeys, size_t buffer_size) override
    {
        if (m_hotkey.key)
        {
            if (hotkeys && buffer_size >= 1)
            {
                hotkeys[0] = m_hotkey;
            }

            return 1;
        }
        else
        {
            return 0;
        }
    }

    // Returns whether the PowerToys should be enabled by default
    virtual bool is_enabled_by_default() const override
    {
        return false;
    }

};

// Load the settings file.
void MouseJump::init_settings()
{
    try
    {
        // Load and parse the settings file for this PowerToy.
        PowerToysSettings::PowerToyValues settings =
            PowerToysSettings::PowerToyValues::load_from_settings_file(MouseJump::get_name());

        parse_hotkey(settings);

    }
    catch (std::exception&)
    {
        Logger::warn(L"An exception occurred while loading the settings file");
        // Error while loading from the settings file. Let default values stay as they are.
    }
}


extern "C" __declspec(dllexport) PowertoyModuleIface* __cdecl powertoy_create()
{
    return new MouseJump();
}