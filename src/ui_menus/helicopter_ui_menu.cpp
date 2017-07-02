#include "../deps/imgui/imgui.h"

#include <jc3/entities/character.h>
#include <jc3/entities/vehicle.h>

#include <json.hpp>

#include <jc3/hashes/vehicles.h>

nlohmann::json HelicopterSettingsToJson(boost::shared_ptr<jc3::CVehicle> vehicle) {
    auto pfxVehicle = vehicle->PfxVehicle;
    assert(pfxVehicle->GetType() == jc3::PfxType::Helicopter && "This vehicle is not a car");
    auto pfxHelicopter = static_cast<jc3::CPfxHelicopter*>(pfxVehicle);

    namespace json = nlohmann;
    json::json settings_json;

    auto model = pfxHelicopter->helicopterModelResourceCachePointer.data;
    auto steering = pfxHelicopter->helicopterSteeringResourceCachePtr.data;
    settings_json["helicopter"] = {
        { "model",{
            { "center_of_torques_x", model->center_of_torques_x },
            { "center_of_torques_y", model->center_of_torques_y },
            { "center_of_torques_z", model->center_of_torques_z },
            { "altitude_input_power", model->altitude_input_power },
            { "yaw_input_power", model->yaw_input_power },
            { "pitch_input_power", model->pitch_input_power },
            { "roll_input_power", model->roll_input_power },
            { "pitch_input_dead_zone", model->pitch_input_dead_zone },
            { "t_to_full_yaw_s", model->t_to_full_yaw_s },
            { "max_speed_t_to_full_yaw_s", model->max_speed_t_to_full_yaw_s },
            { "bank_start_velocity_kmph", model->bank_start_velocity_kmph },
            { "bank_max_velocity_kmph", model->bank_max_velocity_kmph },
            { "min_speed_dive_kmph", model->min_speed_dive_kmph },
            { "max_speed_dive_kmph", model->max_speed_dive_kmph },
            { "add_dive_pitch_deg", model->add_dive_pitch_deg },
            { "add_climb_pitch_deg", model->add_climb_pitch_deg },
            { "max_roll_input_for_climb", model->max_roll_input_for_climb },
            { "climb_speed_low_speed_kmph", model->climb_speed_low_speed_kmph },
            { "dive_speed_low_speed_kmph", model->dive_speed_low_speed_kmph },
            { "min_altitude_input", model->min_altitude_input },
            { "unsettled_altitude_gain_climb", model->unsettled_altitude_gain_climb },
            { "unsettled_altitude_gain_dive", model->unsettled_altitude_gain_dive },
            { "max_diving_gs", model->max_diving_gs },
            { "max_climbing_gs", model->max_climbing_gs },
            { "add_force_forward_power", model->add_force_forward_power },
            { "add_force_lateral_power", model->add_force_lateral_power },
            { "trim_input_gain", model->trim_input_gain },
            { "forward_drag", model->forward_drag },
            { "lateral_drag", model->lateral_drag },
            { "vertical_drag", model->vertical_drag },
            { "tail_lateral_drag", model->tail_lateral_drag },
            { "tail_vertical_drag", model->tail_vertical_drag },
            { "angular_drag", model->angular_drag },
            { "low_speed_max_drag_yaw_speed", model->low_speed_max_drag_yaw_speed },
            { "high_speed_max_drag_yaw_speed", model->high_speed_max_drag_yaw_speed },
            { "yaw_drag_no_input", model->yaw_drag_no_input },
            { "forward_drag_no_input", model->forward_drag_no_input },
            { "lateral_drag_no_input", model->lateral_drag_no_input },
            { "vertical_drag_no_input", model->vertical_drag_no_input },
            { "tail_distance_to_com_m", model->tail_distance_to_com_m },
            { "add_forward_force", model->add_forward_force },
            { "add_right_force", model->add_right_force },
            { "add_lateral_factor_pull_up", model->add_lateral_factor_pull_up },
            { "max_roll_deg", model->max_roll_deg },
            { "add_bank_roll_deg", model->add_bank_roll_deg },
            { "add_bank_roll_pull_up_deg", model->add_bank_roll_pull_up_deg },
            { "max_pitch_low_speed_deg", model->max_pitch_low_speed_deg },
            { "max_pitch_high_speed_deg", model->max_pitch_high_speed_deg },
            { "counter_pitch_angle_deg", model->counter_pitch_angle_deg },
            { "counter_pitch_speed_kmph", model->counter_pitch_speed_kmph },
            { "roll_p", model->roll_p },
            { "roll_i", model->roll_i },
            { "roll_d", model->roll_d },
            { "roll_max_amplitude", model->roll_max_amplitude },
            { "pitch_p", model->pitch_p },
            { "pitch_i", model->pitch_i },
            { "pitch_d", model->pitch_d },
            { "pitch_max_amplitude", model->pitch_max_amplitude },
            { "yaw_p", model->yaw_p },
            { "yaw_i", model->yaw_i },
            { "yaw_d", model->yaw_d },
            { "yaw_max_amplitude", model->yaw_max_amplitude },
            { "low_speed_altitude_p", model->low_speed_altitude_p },
            { "low_speed_altitude_i", model->low_speed_altitude_i },
            { "low_speed_altitude_d", model->low_speed_altitude_d },
            { "high_speed_altitude_p", model->high_speed_altitude_p },
            { "high_speed_altitude_i", model->high_speed_altitude_i },
            { "high_speed_altitude_d", model->high_speed_altitude_d },
            { "altitude_limit_threshold_low", model->altitude_limit_threshold_low },
            { "altitude_limit_threshold_high", model->altitude_limit_threshold_high },
        } },
        { "steering",{
            { "return_pitch_limit", steering->return_pitch_limit },
            { "return_roll_limit", steering->return_roll_limit },
            { "air_steering",{
                { "referenceMaxSpeedKPH", steering->air_steering.referenceMaxSpeedKPH },
                { "referenceMinSpeedKPH", steering->air_steering.referenceMinSpeedKPH },
                { "acceleration_smoothing", steering->air_steering.acceleration_smoothing },
                { "max_steering_angle", steering->air_steering.max_steering_angle },
                { "pitch_return", steering->air_steering.pitch_return },
                { "roll_return", steering->air_steering.roll_return },
                { "rollAxisTiming",{
                    { "timeToMaxInputAtMinSpeed_s", steering->air_steering.rollAxisTiming.timeToMaxInputAtMinSpeed_s },
                    { "timeToMaxInputAtMaxSpeed_s", steering->air_steering.rollAxisTiming.timeToMaxInputAtMaxSpeed_s },
                    { "centeringInputTimeFactor", steering->air_steering.rollAxisTiming.centeringInputTimeFactor },
                    { "counterInputTimeFactor", steering->air_steering.rollAxisTiming.counterInputTimeFactor },
                } },
                { "pitchAxisTiming",{
                    { "timeToMaxInputAtMinSpeed_s", steering->air_steering.pitchAxisTiming.timeToMaxInputAtMinSpeed_s },
                    { "timeToMaxInputAtMaxSpeed_s", steering->air_steering.pitchAxisTiming.timeToMaxInputAtMaxSpeed_s },
                    { "centeringInputTimeFactor", steering->air_steering.pitchAxisTiming.centeringInputTimeFactor },
                    { "counterInputTimeFactor", steering->air_steering.pitchAxisTiming.counterInputTimeFactor },
                } },
                { "yawAxisTiming",{
                    { "timeToMaxInputAtMinSpeed_s", steering->air_steering.yawAxisTiming.timeToMaxInputAtMinSpeed_s },
                    { "timeToMaxInputAtMaxSpeed_s", steering->air_steering.yawAxisTiming.timeToMaxInputAtMaxSpeed_s },
                    { "centeringInputTimeFactor", steering->air_steering.yawAxisTiming.centeringInputTimeFactor },
                    { "counterInputTimeFactor", steering->air_steering.yawAxisTiming.counterInputTimeFactor },
                } },
            } }
        } },
    };

    return settings_json;
}

void HelicopterSettingsFromJson(boost::shared_ptr<jc3::CVehicle> vehicle, nlohmann::json settings_json) {

}

void DoHelicopterHandlingUI(boost::shared_ptr<jc3::CVehicle> real_vehicle, jc3::CPfxVehicle *pfxVehicle) {
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
