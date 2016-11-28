#include "../deps/imgui/imgui.h"

#include <jc3/entities/character.h>
#include <jc3/entities/vehicle.h>

#include <json.hpp>

#include <jc3/hashes/vehicles.h>

void DoHelicopterHandlingUI(jc3::CVehicle *real_vehicle, jc3::CPfxVehicle *pfxVehicle) {
    auto pfxBike = static_cast<jc3::CPfxMotorBike*>(pfxVehicle);

    using json = nlohmann::json;
    static json vehicle_hashes = json::parse(jc3::vehicle_hashes);
    assert(vehicle_hashes.is_array() && "Vehicle hashes is not an array");
    assert(pfxVehicle->GetType() == jc3::PfxType::Helicopter && "This is not a Helicopter");

    // Car stuff
    auto hash = real_vehicle->GetNameHash();

    for (auto &vehicle : vehicle_hashes) {
        if (vehicle["hash"].is_number() && static_cast<uint32_t>(vehicle["hash"]) == hash) {
            std::string t = vehicle["model_name"];
            ImGui::BulletText("Model Name: %s", t.c_str());
        }
    }
    // NOTE(xforce): Helicopter is kinda weird, fuck my life.
}