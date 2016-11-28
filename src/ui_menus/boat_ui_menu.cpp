#include "../deps/imgui/imgui.h"

#include <jc3/entities/character.h>
#include <jc3/entities/vehicle.h>

#include <json.hpp>

#include <jc3/hashes/vehicles.h>

void DoBoatHandlingUI(jc3::CVehicle *real_vehicle, jc3::CPfxVehicle *pfxVehicle) {
    auto pfxBoat = static_cast<jc3::CPfxBoat*>(pfxVehicle);

    using json = nlohmann::json;
    static json vehicle_hashes = json::parse(jc3::vehicle_hashes);
    assert(vehicle_hashes.is_array() && "Vehicle hashes is not an array");
    assert(pfxVehicle->GetType() == jc3::PfxType::Boat && "This is not a Boat");

    // Car stuff
    auto hash = real_vehicle->GetNameHash();

    for (auto &vehicle : vehicle_hashes) {
        if (vehicle["hash"].is_number() && static_cast<uint32_t>(vehicle["hash"]) == hash) {
            std::string t = vehicle["model_name"];
            ImGui::BulletText("Model Name: %s", t.c_str());
        }
    }

    if (ImGui::CollapsingHeader("Propellers")) {
        ImGui::TreePush("Propellers");
        ImGui::DragFloat("max_thrust", &pfxBoat->propellersProperties->max_thrust);
        ImGui::DragFloat("max_rpm", &pfxBoat->propellersProperties->max_rpm);
        ImGui::DragFloat("max_reverse_rpm", &pfxBoat->propellersProperties->max_reverse_rpm);
        ImGui::DragFloat("diameter", &pfxBoat->propellersProperties->diameter);
        ImGui::DragFloat("pitch", &pfxBoat->propellersProperties->pitch);
        ImGui::TreePush("Docking Controls");
        ImGui::DragFloat("optimal_docking_speed_ms", &pfxBoat->propellersProperties->docking_controls.optimal_docking_speed_ms);
        ImGui::DragFloat("max_docking_speed_ms", &pfxBoat->propellersProperties->docking_controls.max_docking_speed_ms);
        ImGui::DragFloat("max_docking_control_throttle", &pfxBoat->propellersProperties->docking_controls.max_docking_control_throttle);
        ImGui::DragFloat("docking_yaw_throttle_limit", &pfxBoat->propellersProperties->docking_controls.docking_yaw_throttle_limit);
        //ImGui::TreePush("Propellers...");
        //if (ImGui::CollapsingHeader("Propeller 1")) {
        //    ImGui::DragFloat("meow", &pfxBoat->propellersProperties->docking_controls.optimal_docking_speed_ms);
        //    ImGui::DragFloat("meow", &pfxBoat->propellersProperties->docking_controls.max_docking_speed_ms);
        //    ImGui::DragFloat("meow", &pfxBoat->propellersProperties->docking_controls.max_docking_control_throttle);
        //    ImGui::DragFloat("meow", &pfxBoat->propellersProperties->docking_controls.docking_yaw_throttle_limit);
        //}
        ImGui::TreePop();
    }
}