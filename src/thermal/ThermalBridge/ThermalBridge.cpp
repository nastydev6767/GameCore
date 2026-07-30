// FILE : src/thermal/ThermalBridge/ThermalBridge.cpp
// Compiled with /clr (C++/CLI)
// ─────────────────────────────────────────────────────────────────────────────
// Confirmed from MSVC error messages:
//   Computer::Hardware    → IList<IHardware^>^   (use ->default[i], ->Count)
//   IHardware::SubHardware → array<IHardware^>^  (use [i], ->Length)
//   IHardware::Sensors    → array<ISensor^>^     (use [i], ->Length)
// ─────────────────────────────────────────────────────────────────────────────

#include "ThermalBridge.h"

#using <mscorlib.dll>
#using <System.dll>
#using "LibreHardwareMonitorLib.dll"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace LibreHardwareMonitor::Hardware;

// ─────────────────────────────────────────────────────────────────────────────
// Managed singleton
// ─────────────────────────────────────────────────────────────────────────────

ref class ThermalState sealed {
public:
    Computer^        Comp;
    List<ISensor^>^  CpuTemps;
    List<ISensor^>^  GpuTemps;
    List<ISensor^>^  CpuFans;
    List<ISensor^>^  GpuFans;
    List<ISensor^>^  CpuLoads;
    List<ISensor^>^  GpuLoads;
    List<ISensor^>^  GpuVrams;
    List<IControl^>^ Controls;
    String^          Manufacturer;
    String^          Method;
    bool             Ready;

    static ThermalState^ GetInstance() {
        if (s_inst == nullptr)
            s_inst = gcnew ThermalState();
        return s_inst;
    }
    static void Reset() { s_inst = nullptr; }

private:
    ThermalState()
        : Comp(nullptr)
        , CpuTemps(gcnew List<ISensor^>())
        , GpuTemps(gcnew List<ISensor^>())
        , CpuFans(gcnew List<ISensor^>())
        , GpuFans(gcnew List<ISensor^>())
        , CpuLoads(gcnew List<ISensor^>())
        , GpuLoads(gcnew List<ISensor^>())
        , GpuVrams(gcnew List<ISensor^>())
        , Controls(gcnew List<IControl^>())
        , Manufacturer(gcnew String("Unknown"))
        , Method(gcnew String("lhm_auto"))
        , Ready(false)
    {}
    static ThermalState^ s_inst = nullptr;
};

// ─────────────────────────────────────────────────────────────────────────────
// ProcessSensors — accepts array<ISensor^>^ (what IHardware::Sensors returns)
// ─────────────────────────────────────────────────────────────────────────────

static void ProcessSensors(ThermalState^ s,
                           array<ISensor^>^ sens,
                           HardwareType ht)
{
    if (sens == nullptr) return;

    bool isCpu = (ht == HardwareType::Cpu);
    bool isGpu = (ht == HardwareType::GpuNvidia ||
                  ht == HardwareType::GpuAmd    ||
                  ht == HardwareType::GpuIntel);
    bool isMb  = (ht == HardwareType::Motherboard ||
                  ht == HardwareType::SuperIO);

    for (int k = 0; k < sens->Length; k++) {
        ISensor^   sn = sens[k];
        SensorType st = sn->SensorType;

        if (sn->Control != nullptr)
            s->Controls->Add(sn->Control);

        if (isCpu) {
            if (st == SensorType::Temperature &&
                sn->Name->Contains("Package"))
                s->CpuTemps->Add(sn);
            else if (st == SensorType::Fan)
                s->CpuFans->Add(sn);
            else if (st == SensorType::Load &&
                     sn->Name->Contains("Total"))
                s->CpuLoads->Add(sn);
        }
        else if (isGpu) {
            if (st == SensorType::Temperature)
                s->GpuTemps->Add(sn);
            else if (st == SensorType::Fan)
                s->GpuFans->Add(sn);
            else if (st == SensorType::Load &&
                     sn->Name->Contains("Core"))
                s->GpuLoads->Add(sn);
            else if (st == SensorType::SmallData &&
                     sn->Name->Contains("Memory Used"))
                s->GpuVrams->Add(sn);
        }
        else if (isMb) {
            if (st == SensorType::Fan)
                s->CpuFans->Add(sn);
        }
    }
}

// ─────────────────────────────────────────────────────────────────────────────
// DoUpdateAll
// Computer::Hardware  → IList<IHardware^>^  → ->default[i], ->Count
// IHardware::SubHardware → array<IHardware^>^ → [i], ->Length
// ─────────────────────────────────────────────────────────────────────────────

static void DoUpdateAll()
{
    ThermalState^ s = ThermalState::GetInstance();
    if (s->Comp == nullptr) return;

    IList<IHardware^>^ hw = s->Comp->Hardware;
    for (int i = 0; i < hw->Count; i++) {
        IHardware^ h = hw->default[i];
        h->Update();
        array<IHardware^>^ subs = h->SubHardware;
        for (int j = 0; j < subs->Length; j++)
            subs[j]->Update();
    }
}

static void DoBuildSensorLists()
{
    ThermalState^ s = ThermalState::GetInstance();
    List<String^>^ methods = gcnew List<String^>();

    IList<IHardware^>^ hw = s->Comp->Hardware;
    for (int i = 0; i < hw->Count; i++)
    {
        IHardware^   h  = hw->default[i];
        HardwareType ht = h->HardwareType;

        // Top-level sensors — array<ISensor^>^
        ProcessSensors(s, h->Sensors, ht);

        // Sub-hardware — array<IHardware^>^
        array<IHardware^>^ subs = h->SubHardware;
        for (int j = 0; j < subs->Length; j++)
            ProcessSensors(s, subs[j]->Sensors, subs[j]->HardwareType);

        // Detect method from identifier
        String^ id = h->Identifier->ToString()->ToLower();
        if      (id->Contains("wmi"))                                 methods->Add("lhm_wmi");
        else if (id->Contains("smm"))                                 methods->Add("lhm_smm");
        else if (id->Contains("ec"))                                  methods->Add("lhm_ec");
        else if (id->Contains("superio") || id->Contains("ite") ||
                 id->Contains("nuvoton"))                             methods->Add("lhm_superio");
        else if (id->Contains("hid"))                                 methods->Add("lhm_hid");
    }

    s->Method = (methods->Count > 0) ? methods[0] : gcnew String("lhm_auto");
}

// ─────────────────────────────────────────────────────────────────────────────
// Helpers
// ─────────────────────────────────────────────────────────────────────────────

static float FirstValue(List<ISensor^>^ list)
{
    if (list == nullptr) return -1.f;
    for (int i = 0; i < list->Count; i++)
        if (list[i]->Value.HasValue)
            return static_cast<float>(list[i]->Value.Value);
    return -1.f;
}

static void CopyStr(String^ src, char* buf, int len)
{
    if (!buf || len <= 0) return;
    IntPtr ptr = Marshal::StringToHGlobalAnsi(src);
    strncpy_s(buf, len, static_cast<char*>(ptr.ToPointer()), len - 1);
    buf[len - 1] = '\0';
    Marshal::FreeHGlobal(ptr);
}

// ─────────────────────────────────────────────────────────────────────────────
// Exported C functions
// ─────────────────────────────────────────────────────────────────────────────

extern "C" {

bool __cdecl GC_Thermal_Init()
{
    try {
        ThermalState^ s = ThermalState::GetInstance();
        s->Comp = gcnew Computer();
        s->Comp->IsCpuEnabled         = true;
        s->Comp->IsGpuEnabled         = true;
        s->Comp->IsMotherboardEnabled  = true;
        s->Comp->IsControllerEnabled   = true;
        s->Comp->IsStorageEnabled      = false;
        s->Comp->IsNetworkEnabled      = false;
        s->Comp->IsBatteryEnabled      = false;
        s->Comp->Open();

        IList<IHardware^>^ hw = s->Comp->Hardware;
        for (int i = 0; i < hw->Count; i++) {
            if (hw->default[i]->HardwareType == HardwareType::Motherboard) {
                s->Manufacturer = hw->default[i]->Name;
                break;
            }
        }

        DoBuildSensorLists();
        DoUpdateAll();
        s->Ready = (hw->Count > 0);
        return s->Ready;
    }
    catch (Exception^) { return false; }
}

float __cdecl GC_Thermal_CpuTemp()
{
    ThermalState^ s = ThermalState::GetInstance();
    if (!s->Ready) return -1.f;
    DoUpdateAll();
    return FirstValue(s->CpuTemps);
}

float __cdecl GC_Thermal_GpuTemp()
{
    ThermalState^ s = ThermalState::GetInstance();
    if (!s->Ready) return -1.f;
    DoUpdateAll();
    return FirstValue(s->GpuTemps);
}

float __cdecl GC_Thermal_CpuFanRpm()
{
    ThermalState^ s = ThermalState::GetInstance();
    if (!s->Ready) return -1.f;
    return FirstValue(s->CpuFans);
}

float __cdecl GC_Thermal_GpuFanRpm()
{
    ThermalState^ s = ThermalState::GetInstance();
    if (!s->Ready) return -1.f;
    return FirstValue(s->GpuFans);
}

float __cdecl GC_Thermal_CpuLoad()
{
    ThermalState^ s = ThermalState::GetInstance();
    if (!s->Ready) return -1.f;
    return FirstValue(s->CpuLoads);
}

float __cdecl GC_Thermal_GpuLoad()
{
    ThermalState^ s = ThermalState::GetInstance();
    if (!s->Ready) return -1.f;
    return FirstValue(s->GpuLoads);
}

float __cdecl GC_Thermal_GpuVramUsedMb()
{
    ThermalState^ s = ThermalState::GetInstance();
    if (!s->Ready) return -1.f;
    return FirstValue(s->GpuVrams);
}

bool __cdecl GC_Thermal_MaximizeAll(int* controllersFound)
{
    ThermalState^ s = ThermalState::GetInstance();
    if (!s->Ready || s->Controls == nullptr) {
        if (controllersFound) *controllersFound = 0;
        return false;
    }
    int count = 0;
    for (int i = 0; i < s->Controls->Count; i++) {
        try { s->Controls[i]->SetSoftware(100.0f); ++count; }
        catch (Exception^) {}
    }
    if (controllersFound) *controllersFound = count;
    return count > 0;
}

void __cdecl GC_Thermal_RestoreAll()
{
    ThermalState^ s = ThermalState::GetInstance();
    if (!s->Ready || s->Controls == nullptr) return;
    for (int i = 0; i < s->Controls->Count; i++) {
        try { s->Controls[i]->SetDefault(); }
        catch (Exception^) {}
    }
}

void __cdecl GC_Thermal_GetManufacturer(char* buf, int len)
{
    CopyStr(ThermalState::GetInstance()->Manufacturer, buf, len);
}

void __cdecl GC_Thermal_GetMethod(char* buf, int len)
{
    CopyStr(ThermalState::GetInstance()->Method, buf, len);
}

void __cdecl GC_Thermal_Shutdown()
{
    ThermalState^ s = ThermalState::GetInstance();
    if (s->Comp != nullptr) {
        try { s->Comp->Close(); } catch (Exception^) {}
        s->Comp  = nullptr;
        s->Ready = false;
    }
    ThermalState::Reset();
}

} // extern "C"