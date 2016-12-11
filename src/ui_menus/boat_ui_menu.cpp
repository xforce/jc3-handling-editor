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
        ImGui::DragFloat("max_thrust", &pfxBoat->propellersResourceCachePtr.data->max_thrust);
        ImGui::DragFloat("max_rpm", &pfxBoat->propellersResourceCachePtr.data->max_rpm);
        ImGui::DragFloat("max_reverse_rpm", &pfxBoat->propellersResourceCachePtr.data->max_reverse_rpm);
        ImGui::DragFloat("diameter", &pfxBoat->propellersResourceCachePtr.data->diameter);
        ImGui::DragFloat("pitch", &pfxBoat->propellersResourceCachePtr.data->pitch);
        ImGui::TreePush("Docking Controls");
        ImGui::DragFloat("optimal_docking_speed_ms", &pfxBoat->propellersResourceCachePtr.data->docking_controls.optimal_docking_speed_ms);
        ImGui::DragFloat("max_docking_speed_ms", &pfxBoat->propellersResourceCachePtr.data->docking_controls.max_docking_speed_ms);
        ImGui::DragFloat("max_docking_control_throttle", &pfxBoat->propellersResourceCachePtr.data->docking_controls.max_docking_control_throttle);
        ImGui::DragFloat("docking_yaw_throttle_limit", &pfxBoat->propellersResourceCachePtr.data->docking_controls.docking_yaw_throttle_limit);
        ImGui::TreePop();
        ImGui::TreePop();
    }

    if (pfxBoat->finsResourceCachePtr.data && ImGui::CollapsingHeader("Fins")) {
        ImGui::TreePush("Fins");
        ImGui::DragFloat("reference_speed_ms", &pfxBoat->finsResourceCachePtr.data->reference_speed_ms);
        ImGui::DragFloat("pressure_drag", &pfxBoat->finsResourceCachePtr.data->pressure_drag);
        ImGui::DragFloat("pressure_drag2", &pfxBoat->finsResourceCachePtr.data->pressure_drag2);
        ImGui::TreePop();
    }

    if (ImGui::CollapsingHeader("Steering")) {
        ImGui::TreePush("Steering");
        ImGui::DragFloat("acceleration_smoothing", &pfxBoat->boatSteeringResourceCachePtr.data->acceleration_smoothing);
        if (ImGui::CollapsingHeader("Steering Filter")) {
            ImGui::TreePush("Steering Filter");

            auto &steeringFilter = pfxBoat->boatSteeringResourceCachePtr.data->steeringfilter;
            ImGui::DragFloat("t_to_full_input_min_speed_s", &steeringFilter.t_to_full_input_min_speed_s);
            ImGui::DragFloat("t_to_full_input_max_speed_s", &steeringFilter.t_to_full_input_max_speed_s);
            ImGui::DragFloat("input_start_speed_kmph", &steeringFilter.input_start_speed_kmph);
            ImGui::DragFloat("input_max_speed_kmph", &steeringFilter.input_max_speed_kmph);
            ImGui::DragFloat("counterinput_speed_factor", &steeringFilter.counterinput_speed_factor);
            ImGui::DragFloat("zeroinput_speed_factor", &steeringFilter.zeroinput_speed_factor);
            ImGui::DragFloat("input_speedcurve_falloff", &steeringFilter.input_speedcurve_falloff);

            pfxBoat->ApplyBoatSteering(*pfxBoat->boatSteeringResourceCachePtr.data);
            ImGui::TreePop();
        }
        ImGui::TreePop();
    }
}