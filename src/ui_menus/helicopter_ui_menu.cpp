#include "../deps/imgui/imgui.h"

#include <jc3/entities/character.h>
#include <jc3/entities/vehicle.h>

#include <json.hpp>

#include <jc3/hashes/vehicles.h>

void DoHelicopterHandlingUI(jc3::CVehicle *real_vehicle, jc3::CPfxVehicle *pfxVehicle) {
    auto pfxHelicopter = static_cast<jc3::CPfxHelicopter*>(pfxVehicle);

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
    if (ImGui::CollapsingHeader("Model")) {
        ImGui::TreePush("Model");
        auto model = pfxHelicopter->helicopterModelResourceCachePointer.data;
        ImGui::DragFloat("center_of_torques_x", &model->center_of_torques_x);
        ImGui::DragFloat("center_of_torques_y", &model->center_of_torques_y);
        ImGui::DragFloat("center_of_torques_z", &model->center_of_torques_z);
        ImGui::DragFloat("altitude_input_power", &model->altitude_input_power);
        ImGui::DragFloat("yaw_input_power", &model->yaw_input_power);
        ImGui::DragFloat("pitch_input_power", &model->pitch_input_power);
        ImGui::DragFloat("roll_input_power", &model->roll_input_power);
        ImGui::DragFloat("pitch_input_dead_zone", &model->pitch_input_dead_zone);
        ImGui::DragFloat("t_to_full_yaw_s", &model->t_to_full_yaw_s);
        ImGui::DragFloat("max_speed_t_to_full_yaw_s", &model->max_speed_t_to_full_yaw_s);
        ImGui::DragFloat("bank_start_velocity_kmph", &model->bank_start_velocity_kmph);
        ImGui::DragFloat("bank_max_velocity_kmph", &model->bank_max_velocity_kmph);
        ImGui::DragFloat("min_speed_dive_kmph", &model->min_speed_dive_kmph);
        ImGui::DragFloat("max_speed_dive_kmph", &model->max_speed_dive_kmph);
        ImGui::DragFloat("add_dive_pitch_deg", &model->add_dive_pitch_deg);
        ImGui::DragFloat("add_climb_pitch_deg", &model->add_climb_pitch_deg);
        ImGui::DragFloat("max_roll_input_for_climb", &model->max_roll_input_for_climb);
        ImGui::DragFloat("climb_speed_low_speed_kmph", &model->climb_speed_low_speed_kmph);
        ImGui::DragFloat("dive_speed_low_speed_kmph", &model->dive_speed_low_speed_kmph);
        ImGui::DragFloat("min_altitude_input", &model->min_altitude_input);
        ImGui::DragFloat("unsettled_altitude_gain_climb", &model->unsettled_altitude_gain_climb);
        ImGui::DragFloat("unsettled_altitude_gain_dive", &model->unsettled_altitude_gain_dive);
        ImGui::DragFloat("max_diving_gs", &model->max_diving_gs);
        ImGui::DragFloat("max_climbing_gs", &model->max_climbing_gs);
        ImGui::DragFloat("add_force_forward_power", &model->add_force_forward_power);
        ImGui::DragFloat("add_force_lateral_power", &model->add_force_lateral_power);
        ImGui::DragFloat("trim_input_gain", &model->trim_input_gain);
        ImGui::DragFloat("forward_drag", &model->forward_drag);
        ImGui::DragFloat("lateral_drag", &model->lateral_drag);
        ImGui::DragFloat("vertical_drag", &model->vertical_drag);
        ImGui::DragFloat("tail_lateral_drag", &model->tail_lateral_drag);
        ImGui::DragFloat("tail_vertical_drag", &model->tail_vertical_drag);
        ImGui::DragFloat("angular_drag", &model->angular_drag);
        ImGui::DragFloat("low_speed_max_drag_yaw_speed", &model->low_speed_max_drag_yaw_speed);
        ImGui::DragFloat("high_speed_max_drag_yaw_speed", &model->high_speed_max_drag_yaw_speed);
        ImGui::DragFloat("yaw_drag_no_input", &model->yaw_drag_no_input);
        ImGui::DragFloat("forward_drag_no_input", &model->forward_drag_no_input);
        ImGui::DragFloat("lateral_drag_no_input", &model->lateral_drag_no_input);
        ImGui::DragFloat("vertical_drag_no_input", &model->vertical_drag_no_input);
        ImGui::DragFloat("tail_distance_to_com_m", &model->tail_distance_to_com_m);
        ImGui::DragFloat("add_forward_force", &model->add_forward_force);
        ImGui::DragFloat("add_right_force", &model->add_right_force);
        ImGui::DragFloat("add_lateral_factor_pull_up", &model->add_lateral_factor_pull_up);
        ImGui::DragFloat("max_roll_deg", &model->max_roll_deg);
        ImGui::DragFloat("add_bank_roll_deg", &model->add_bank_roll_deg);
        ImGui::DragFloat("add_bank_roll_pull_up_deg", &model->add_bank_roll_pull_up_deg);
        ImGui::DragFloat("max_pitch_low_speed_deg", &model->max_pitch_low_speed_deg);
        ImGui::DragFloat("max_pitch_high_speed_deg", &model->max_pitch_high_speed_deg);
        ImGui::DragFloat("counter_pitch_angle_deg", &model->counter_pitch_angle_deg);
        ImGui::DragFloat("counter_pitch_speed_kmph", &model->counter_pitch_speed_kmph);
        ImGui::DragFloat("roll_p", &model->roll_p);
        ImGui::DragFloat("roll_i", &model->roll_i);
        ImGui::DragFloat("roll_d", &model->roll_d);
        ImGui::DragFloat("roll_max_amplitude", &model->roll_max_amplitude);
        ImGui::DragFloat("pitch_p", &model->pitch_p);
        ImGui::DragFloat("pitch_i", &model->pitch_i);
        ImGui::DragFloat("pitch_d", &model->pitch_d);
        ImGui::DragFloat("pitch_max_amplitude", &model->pitch_max_amplitude);
        ImGui::DragFloat("yaw_p", &model->yaw_p);
        ImGui::DragFloat("yaw_i", &model->yaw_i);
        ImGui::DragFloat("yaw_d", &model->yaw_d);
        ImGui::DragFloat("yaw_max_amplitude", &model->yaw_max_amplitude);
        ImGui::DragFloat("low_speed_altitude_p", &model->low_speed_altitude_p);
        ImGui::DragFloat("low_speed_altitude_i", &model->low_speed_altitude_i);
        ImGui::DragFloat("low_speed_altitude_d", &model->low_speed_altitude_d);
        ImGui::DragFloat("high_speed_altitude_p", &model->high_speed_altitude_p);
        ImGui::DragFloat("high_speed_altitude_i", &model->high_speed_altitude_i);
        ImGui::DragFloat("high_speed_altitude_d", &model->high_speed_altitude_d);
        ImGui::DragFloat("altitude_limit_threshold_low", &model->altitude_limit_threshold_low);
        ImGui::DragFloat("altitude_limit_threshold_high", &model->altitude_limit_threshold_high);
        ImGui::TreePop();

        pfxHelicopter->ApplyHelicopterModel(*model);
    }

    if (ImGui::CollapsingHeader("Steering")) {
        ImGui::TreePush("Steering");
        auto steering = pfxHelicopter->helicopterSteeringResourceCachePtr.data;

        if (ImGui::CollapsingHeader("Air Steering")) {
            ImGui::TreePush("Air Steering");

            ImGui::DragFloat("referenceMaxSpeedKPH", &steering->air_steering.referenceMaxSpeedKPH);
            ImGui::DragFloat("referenceMinSpeedKPH", &steering->air_steering.referenceMinSpeedKPH);
            ImGui::DragFloat("min_thrust", &steering->air_steering.acceleration_smoothing);
            ImGui::DragFloat("run_thrust", &steering->air_steering.max_steering_angle);
            ImGui::DragFloat("pitch_return", &steering->air_steering.pitch_return);
            ImGui::DragFloat("roll_return", &steering->air_steering.roll_return);
            ImGui::Separator();
            ImGui::TreePush("rollAxisTiming");
            ImGui::Text("rollAxisTiming");
            ImGui::DragFloat("timeToMaxInputAtMinSpeed_s", &steering->air_steering.rollAxisTiming.timeToMaxInputAtMinSpeed_s);
            ImGui::DragFloat("timeToMaxInputAtMaxSpeed_s", &steering->air_steering.rollAxisTiming.timeToMaxInputAtMaxSpeed_s);
            ImGui::DragFloat("centeringInputTimeFactor", &steering->air_steering.rollAxisTiming.centeringInputTimeFactor);
            ImGui::DragFloat("counterInputTimeFactor", &steering->air_steering.rollAxisTiming.counterInputTimeFactor);
            ImGui::TreePop();
            ImGui::Separator();
            ImGui::TreePush("pitchAxisTiming");
            ImGui::Text("pitchAxisTiming");
            ImGui::DragFloat("timeToMaxInputAtMinSpeed_s", &steering->air_steering.pitchAxisTiming.timeToMaxInputAtMinSpeed_s);
            ImGui::DragFloat("timeToMaxInputAtMaxSpeed_s", &steering->air_steering.pitchAxisTiming.timeToMaxInputAtMaxSpeed_s);
            ImGui::DragFloat("centeringInputTimeFactor", &steering->air_steering.pitchAxisTiming.centeringInputTimeFactor);
            ImGui::DragFloat("counterInputTimeFactor", &steering->air_steering.pitchAxisTiming.counterInputTimeFactor);
            ImGui::TreePop();
            ImGui::Separator();
            ImGui::TreePush("yawAxisTiming");
            ImGui::Text("yawAxisTiming");
            ImGui::DragFloat("timeToMaxInputAtMinSpeed_s", &steering->air_steering.yawAxisTiming.timeToMaxInputAtMinSpeed_s);
            ImGui::DragFloat("timeToMaxInputAtMaxSpeed_s", &steering->air_steering.yawAxisTiming.timeToMaxInputAtMaxSpeed_s);
            ImGui::DragFloat("centeringInputTimeFactor", &steering->air_steering.yawAxisTiming.centeringInputTimeFactor);
            ImGui::DragFloat("counterInputTimeFactor", &steering->air_steering.yawAxisTiming.counterInputTimeFactor);
            ImGui::TreePop();

            ImGui::TreePop();
        }

        ImGui::DragFloat("return_pitch_limit", &steering->return_pitch_limit);
        ImGui::DragFloat("return_roll_limit", &steering->return_roll_limit);
        ImGui::TreePop();
        pfxHelicopter->ApplyHelicopterSteering(*steering);
    }
}