#include "../deps/imgui/imgui.h"

#include <jc3/entities/character.h>
#include <jc3/entities/vehicle.h>

#include <json.hpp>

#include <jc3/hashes/vehicles.h>

#include <jc3/entities/pfx/air_audio.h>

void DoPlaneHandlingUI(jc3::CVehicle *real_vehicle, jc3::CPfxVehicle *pfxVehicle) {
    auto pfxAirplane = static_cast<jc3::CPfxAirPlane*>(pfxVehicle);

    using json = nlohmann::json;
    static json vehicle_hashes = json::parse(jc3::vehicle_hashes);
    assert(vehicle_hashes.is_array() && "Vehicle hashes is not an array");
    assert(pfxVehicle->GetType() == jc3::PfxType::Airplane && "This is not a Airplane");

    // Doesn't seem to do anything for planes :(
    ImGui::DragFloat("Top Speed", (float*)((char*)pfxVehicle + 0x3EC));

    /*if (ImGui::CollapsingHeader("Aerodynamics")) 
    }*/

    if (ImGui::CollapsingHeader("Air Engine")) {
        ImGui::TreePush("Air Engine");

        ImGui::DragFloat("max_thrust", &pfxAirplane->airEngineResourceCachePtr.data->max_thrust);
        ImGui::DragFloat("max_thrust_acceleration", &pfxAirplane->airEngineResourceCachePtr.data->max_thrust_acceleration);
        ImGui::DragFloat("min_thrust", &pfxAirplane->airEngineResourceCachePtr.data->min_thrust);
        ImGui::DragFloat("run_thrust", &pfxAirplane->airEngineResourceCachePtr.data->run_thrust);
        ImGui::DragFloat("taxiing_input_threshold", &pfxAirplane->airEngineResourceCachePtr.data->taxiing_input_threshold);
        ImGui::DragFloat("taxiing_max_thrust", &pfxAirplane->airEngineResourceCachePtr.data->taxiing_max_thrust);
        ImGui::DragFloat("taxiing_top_speed", &pfxAirplane->airEngineResourceCachePtr.data->taxiing_top_speed);

        pfxAirplane->ApplyAirEngine(*pfxAirplane->airEngineResourceCachePtr.data);

        ImGui::TreePop();
    }

    if (ImGui::CollapsingHeader("Air Steering")) {
        ImGui::TreePush("Air Steering");

        ImGui::DragFloat("referenceMaxSpeedKPH", &pfxAirplane->airSteeringResourceCachePtr.data->referenceMaxSpeedKPH);
        ImGui::DragFloat("referenceMinSpeedKPH", &pfxAirplane->airSteeringResourceCachePtr.data->referenceMinSpeedKPH);
        ImGui::DragFloat("min_thrust", &pfxAirplane->airSteeringResourceCachePtr.data->acceleration_smoothing);
        ImGui::DragFloat("run_thrust", &pfxAirplane->airSteeringResourceCachePtr.data->max_steering_angle);
        ImGui::DragFloat("pitch_return", &pfxAirplane->airSteeringResourceCachePtr.data->pitch_return);
        ImGui::DragFloat("roll_return", &pfxAirplane->airSteeringResourceCachePtr.data->roll_return);
        ImGui::Separator();
        ImGui::TreePush("rollAxisTiming");
        ImGui::Text("rollAxisTiming");
        ImGui::DragFloat("timeToMaxInputAtMinSpeed_s", &pfxAirplane->airSteeringResourceCachePtr.data->rollAxisTiming.timeToMaxInputAtMinSpeed_s);
        ImGui::DragFloat("timeToMaxInputAtMaxSpeed_s", &pfxAirplane->airSteeringResourceCachePtr.data->rollAxisTiming.timeToMaxInputAtMaxSpeed_s);
        ImGui::DragFloat("centeringInputTimeFactor", &pfxAirplane->airSteeringResourceCachePtr.data->rollAxisTiming.centeringInputTimeFactor);
        ImGui::DragFloat("counterInputTimeFactor", &pfxAirplane->airSteeringResourceCachePtr.data->rollAxisTiming.counterInputTimeFactor);
        ImGui::TreePop();
        ImGui::Separator();
        ImGui::TreePush("pitchAxisTiming");
        ImGui::Text("pitchAxisTiming");
        ImGui::DragFloat("timeToMaxInputAtMinSpeed_s", &pfxAirplane->airSteeringResourceCachePtr.data->pitchAxisTiming.timeToMaxInputAtMinSpeed_s);
        ImGui::DragFloat("timeToMaxInputAtMaxSpeed_s", &pfxAirplane->airSteeringResourceCachePtr.data->pitchAxisTiming.timeToMaxInputAtMaxSpeed_s);
        ImGui::DragFloat("centeringInputTimeFactor", &pfxAirplane->airSteeringResourceCachePtr.data->pitchAxisTiming.centeringInputTimeFactor);
        ImGui::DragFloat("counterInputTimeFactor", &pfxAirplane->airSteeringResourceCachePtr.data->pitchAxisTiming.counterInputTimeFactor);
        ImGui::TreePop();
        ImGui::Separator();
        ImGui::TreePush("yawAxisTiming");
        ImGui::Text("yawAxisTiming");
        ImGui::DragFloat("timeToMaxInputAtMinSpeed_s", &pfxAirplane->airSteeringResourceCachePtr.data->yawAxisTiming.timeToMaxInputAtMinSpeed_s);
        ImGui::DragFloat("timeToMaxInputAtMaxSpeed_s", &pfxAirplane->airSteeringResourceCachePtr.data->yawAxisTiming.timeToMaxInputAtMaxSpeed_s);
        ImGui::DragFloat("centeringInputTimeFactor", &pfxAirplane->airSteeringResourceCachePtr.data->yawAxisTiming.centeringInputTimeFactor);
        ImGui::DragFloat("counterInputTimeFactor", &pfxAirplane->airSteeringResourceCachePtr.data->yawAxisTiming.counterInputTimeFactor);
        ImGui::TreePop();

        pfxAirplane->ApplyAirSteering(*pfxAirplane->airSteeringResourceCachePtr.data);

        ImGui::TreePop();
    }

    // Somehow a jet doesn't have air audio, I don't get it tbh
    //if (ImGui::CollapsingHeader("Air Audio")) {
    //    ImGui::TreePush("Air Audio");
    //    auto &meow = (*(jc3::TAdfStructPtr<jc3::SAirAudio>*)((char*)pfxVehicle + 0x5568));
    //    ImGui::DragFloat("top_speed", &pfxAirplane->airAudioResourceCachePtr.data->top_speed);
    //    ImGui::DragFloat("max_rpm", &pfxAirplane->airAudioResourceCachePtr.data->max_rpm);
    //    ImGui::TreePop();
    //}
}