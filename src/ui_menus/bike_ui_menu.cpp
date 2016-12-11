#include "../deps/imgui/imgui.h"

#include <jc3/entities/character.h>
#include <jc3/entities/vehicle.h>

#include <json.hpp>

#include <jc3/hashes/vehicles.h>

void DoMotorBikeHandlingUI(jc3::CVehicle *real_vehicle, jc3::CPfxVehicle *pfxVehicle) {
	auto pfxBike = static_cast<jc3::CPfxMotorBike*>(pfxVehicle);

	using json = nlohmann::json;
	static json vehicle_hashes = json::parse(jc3::vehicle_hashes);
	assert(vehicle_hashes.is_array() && "Vehicle hashes is not an array");
	assert(pfxVehicle->GetType() == jc3::PfxType::MotorBike && "This is not a Motorbike");

    // Car stuff
    auto hash = real_vehicle->GetNameHash();

    for (auto &vehicle : vehicle_hashes) {
        if (vehicle["hash"].is_number() && static_cast<uint32_t>(vehicle["hash"]) == hash) {
            std::string t = vehicle["model_name"];
            ImGui::BulletText("Model Name: %s", t.c_str());
        }
    }

    ImGui::BulletText("Engine Torque %f", pfxBike->engineTorque);
    ImGui::BulletText("Engine RPM %f", pfxBike->engineRPM);
    ImGui::BulletText("Top Speed %f", pfxBike->topSpeedKph);
    ImGui::Separator();

    ImGui::DragFloat("Top Speed", (float*)((char*)pfxVehicle + 0x3EC));

    ImGui::DragFloat("Drag Coefficient", &real_vehicle->DragCoefficient);
    ImGui::DragFloat("Mass", &real_vehicle->Mass);
    ImGui::DragFloat("Linear Damping", &real_vehicle->LinearDamping);
    ImGui::DragFloat("Angular Damping", &real_vehicle->AngularDamping);
    ImGui::SliderFloat("Gravity Factor", &real_vehicle->GravityFactor, -128, 128);

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Gravity Modifiers")) {
        ImGui::TreePush("Gravity Modifiers");
        ImGui::DragFloat("Gravity Grounded", &pfxBike->someGravityModifiers->gravityMultiplierGrounded);
        ImGui::TreePop();
    }


    if (ImGui::CollapsingHeader("Engine"))
    {
        ImGui::TreePush("Engine");
        auto engine = pfxBike->landEngineResourceCachePtr.data;
        ImGui::DragFloat("resistance_at_min_rpm", &engine->resistance_at_min_rpm);
        ImGui::DragFloat("resistance_at_max_rpm", &engine->resistance_at_max_rpm);
        ImGui::DragFloat("resistance_at_optimal_rpm", &engine->resistance_at_optimal_rpm);
        ImGui::DragFloat("rev_limiter_rpm_drop", &engine->rev_limiter_rpm_drop);
        ImGui::DragFloat("max_rpm", &engine->max_rpm);
        ImGui::DragFloat("min_rpm", &engine->min_rpm);
        ImGui::DragFloat("optimal_rpm", &engine->optimal_rpm);
        ImGui::DragFloat("torque_factor_at_max_rpm", &engine->torque_factor_at_max_rpm);
        ImGui::DragFloat("torque_factor_at_min_rpm", &engine->torque_factor_at_min_rpm);
        ImGui::DragFloat("torque_factor_at_optimal_rpm", &engine->torque_factor_at_optimal_rpm);
        ImGui::DragFloat("clutch_slip_rpm", &engine->clutch_slip_rpm);
        ImGui::DragFloat("engine_min_noise", &engine->engine_min_noise);
        ImGui::DragFloat("engine_damage_noise_scale", &engine->engine_damage_noise_scale);
        ImGui::DragFloat("engine_max_damage_torque", &engine->engine_max_damage_torque);
        ImGui::TreePop();

        pfxBike->ApplyLandEngine(*pfxBike->landEngineResourceCachePtr.data);
    }

    if (ImGui::CollapsingHeader("Engine Transmission")) {
        ImGui::TreePush("Engine Transmission");

        ImGui::DragInt("gears", &pfxBike->transmissionResourceCachePtr.data->gears);
        ImGui::DragInt("nitrous_gears", &pfxBike->transmissionResourceCachePtr.data->nitrous_gears);

        ImGui::DragInt("sequential", &pfxBike->transmissionResourceCachePtr.data->sequential);
        ImGui::DragInt("manual_clutch", &pfxBike->transmissionResourceCachePtr.data->manual_clutch);

        ImGui::DragFloat("manual_clutch_blend_rpm", &pfxBike->transmissionResourceCachePtr.data->manual_clutch_blend_rpm);
        ImGui::DragFloat("manual_clutch_blend_time", &pfxBike->transmissionResourceCachePtr.data->manual_clutch_blend_time);
        ImGui::DragFloat("forward_ratio_percentage", &pfxBike->transmissionResourceCachePtr.data->forward_ratio_percentage);
        ImGui::DragFloat("low_gear_forward_ratio_pct", &pfxBike->transmissionResourceCachePtr.data->low_gear_forward_ratio_pct);

        ImGui::DragFloat("top_speed", &pfxBike->transmissionResourceCachePtr.data->top_speed);
        ImGui::DragFloat("low_gears_final_drive", &pfxBike->transmissionResourceCachePtr.data->low_gears_final_drive);
        ImGui::DragFloat("final_drive", &pfxBike->transmissionResourceCachePtr.data->final_drive);
        ImGui::DragFloat("reverse_gear_ratio", &pfxBike->transmissionResourceCachePtr.data->reverse_gear_ratio);
        ImGui::DragFloat("clutch_delay", &pfxBike->transmissionResourceCachePtr.data->clutch_delay);
        ImGui::DragFloat("decay_time_to_cruise_rpm", &pfxBike->transmissionResourceCachePtr.data->decay_time_to_cruise_rpm);
        ImGui::DragFloat("target_cruise_rpm", &pfxBike->transmissionResourceCachePtr.data->target_cruise_rpm);

        for (int i = 0; i < pfxBike->transmissionResourceCachePtr.data->gears; ++i) {
            char wheel_text[100];
            sprintf(wheel_text, "Wheel Torque Ratio %d", i);
            ImGui::TreePush(wheel_text);
            ImGui::SliderFloat(wheel_text, &pfxBike->transmissionResourceCachePtr.data->gear_ratios[i], 0, 128);
            ImGui::TreePop();
        }
        ImGui::TreePop();

        pfxBike->ApplyTransmission(*pfxBike->transmissionResourceCachePtr.data);
    }

    if (ImGui::CollapsingHeader("Suspension")) {
        ImGui::TreePush("Suspension");
        for (int i = 0; i < pfxBike->wheelInfo.size; ++i) {
            auto & suspension = pfxBike->wheelSuspensionConstants[i];
            char wheel_text[100];
            sprintf(wheel_text, "Suspension Wheel %d", i);
            if (ImGui::CollapsingHeader(wheel_text)) {
                ImGui::TreePush(wheel_text);
                ImGui::DragFloat("Suspension Force Mag At Rest", &suspension.suspensionForceMagnitudeAtRest_N);
                ImGui::DragFloat("Suspension Length At Rest", &suspension.suspensionLengthAtRest_m);
                if (ImGui::CollapsingHeader("Properties")) {

                    auto properties = suspension.suspensionProperties;
                    ImGui::DragFloat("Antirollbar Strength", &properties->antirollbar_strength);
                    ImGui::DragFloat("Compression", &properties->compression);
                    ImGui::DragFloat("Length", &properties->length);
                    ImGui::DragFloat("Relaxation", &properties->relaxation);
                    ImGui::DragFloat("Strength", &properties->strength);
                    ImGui::DragFloat("Lateral Tire Force Offset", &properties->lateral_tire_force_offset);
                    ImGui::DragFloat("Longtid Tire Force Offset", &properties->longitudinal_tire_force_offset);
                    ImGui::DragFloat("Tire drag Force offset", &properties->tire_drag_force_offset);
                    ImGui::DragFloat("Hardpoint offset along spring", &properties->hardpoint_offset_along_spring);
                }
                ImGui::TreePop();
            }
        }

        ImGui::TreePop();
    }

    if (ImGui::CollapsingHeader("Brakes")) {
        ImGui::TreePush("Brakes Front");
        ImGui::Text("Front");
        ImGui::Checkbox("Handbrake", (bool*)&pfxBike->brakesResourceCachePtr.data->front.handbrake);
        ImGui::DragFloat("Max Brake Torque", &pfxBike->brakesResourceCachePtr.data->front.max_brake_torque);
        ImGui::DragFloat("Time To Block", &pfxBike->brakesResourceCachePtr.data->front.min_time_to_block);
        ImGui::TreePop();
        ImGui::Separator();
        ImGui::Text("Rear");
        ImGui::TreePush("Brakes Rear");
        ImGui::Checkbox("Handbrake", (bool*)&pfxBike->brakesResourceCachePtr.data->rear.handbrake);
        ImGui::DragFloat("Max Brake Torque", &pfxBike->brakesResourceCachePtr.data->rear.max_brake_torque);
        ImGui::DragFloat("Time To Block", &pfxBike->brakesResourceCachePtr.data->rear.min_time_to_block);
        pfxBike->ApplyBrakes(*pfxBike->brakesResourceCachePtr.data);
        ImGui::TreePop();
    }

    if (ImGui::CollapsingHeader("Aerodynamics")) {
        ImGui::TreePush("Aerodynamic");
        ImGui::DragFloat("Air Density", &pfxBike->landAerodynamicsResourceCachePtr.data->air_density);
        ImGui::DragFloat("Frontal Area", &pfxBike->landAerodynamicsResourceCachePtr.data->frontal_area);
        ImGui::DragFloat("Drag Coefficient", &pfxBike->landAerodynamicsResourceCachePtr.data->drag_coefficient);
        ImGui::DragFloat("Top Speed Drag Coefficient", &pfxBike->landAerodynamicsResourceCachePtr.data->top_speed_drag_coefficient);
        ImGui::DragFloat("Lift Coefficient", &pfxBike->landAerodynamicsResourceCachePtr.data->lift_coefficient);
        pfxBike->ApplyLandAerodynamics(*pfxBike->landAerodynamicsResourceCachePtr.data);
        ImGui::TreePop();
    }

    if (ImGui::CollapsingHeader("Wheels")) {

        auto DrawWheelInfo = [](const char *id, jc3::WheelInfo * wheelInfo) {
            ImGui::DragFloat("Spin Velocity", &wheelInfo->spinVelocity);
            ImGui::DragFloat("Spin Angle", &wheelInfo->spinAngle);
            ImGui::DragFloat("Side Force", &wheelInfo->sideForce);
            ImGui::DragFloat("Forward Slip Velocity", &wheelInfo->forwardSlipVelocity);
            ImGui::DragFloat("Side Slip Velocity", &wheelInfo->sideSlipVelocity);
            ImGui::DragFloat("Torque", &wheelInfo->torque);
            ImGui::DragFloat("Angular Velocity", &wheelInfo->angularVelocity);
            ImGui::DragFloat("Inv Inertia", &wheelInfo->invInertia);
            ImGui::DragFloat("Slip Angle Deg", &wheelInfo->slipAngleDeg);
            ImGui::DragFloat("Slip Ratio AE", &wheelInfo->slipRatioSAE);
            ImGui::DragFloat("Camber Angle Deg", &wheelInfo->camberAngleDeg);
            ImGui::DragFloat("Lateral Tire Force Offset", &wheelInfo->lateralTireForceOffset);
            ImGui::DragFloat("Longitudinal Tire Force Offset", &wheelInfo->longitudinalTireForceOffset);
            ImGui::DragFloat("Tire Drag Force Offset", &wheelInfo->tireDragForceOffset);
            ImGui::DragFloat("Friction Multiplier", &wheelInfo->wheelFrictionMultiplier);
            ImGui::DragFloat("Drag Multiplier", &wheelInfo->wheelDragMultiplier);
            ImGui::DragFloat("Burnout Friction Multiplier", &wheelInfo->burnoutFrictionMultiplier);
            ImGui::DragFloat("Ground Friction Torque", &wheelInfo->groundFrictionTorque);
            ImGui::DragFloat("Depth of Contact Point Underwater", &wheelInfo->unkown1);
            ImGui::Checkbox("Constrained to Ground", (bool*)&wheelInfo->isConstrainedToGround);
        };

        for (int i = 0; i < pfxBike->wheelInfo.size; ++i) {
            char wheel_text[100];
            sprintf(wheel_text, "Wheel %d", i);
            ImGui::TreePush(wheel_text);
            if (ImGui::CollapsingHeader(wheel_text)) {
                DrawWheelInfo(wheel_text, &pfxBike->wheelInfo.Data[i]);
            }
            ImGui::TreePop();
        }
    }

    if (ImGui::CollapsingHeader("Steering")) {
        ImGui::TreePush("Steering");
        ImGui::DragFloat("dead_zone", &pfxBike->motorbikeSteeringResourceCachePtr.data->land_steering.dead_zone);
        ImGui::DragFloat("saturation_zone", &pfxBike->motorbikeSteeringResourceCachePtr.data->land_steering.saturation_zone);
        ImGui::DragFloat("t_to_full_steer_s", &pfxBike->motorbikeSteeringResourceCachePtr.data->land_steering.t_to_full_steer_s);
        ImGui::DragFloat("max_speed_t_to_full_steer_s", &pfxBike->motorbikeSteeringResourceCachePtr.data->land_steering.max_speed_t_to_full_steer_s);
        ImGui::DragFloat("min_speed_kmph", &pfxBike->motorbikeSteeringResourceCachePtr.data->land_steering.min_speed_kmph);
        ImGui::DragFloat("max_speed_kmph", &pfxBike->motorbikeSteeringResourceCachePtr.data->land_steering.max_speed_kmph);
        ImGui::DragFloat("steer_angle_min_speed_deg", &pfxBike->motorbikeSteeringResourceCachePtr.data->land_steering.steer_angle_min_speed_deg);
        ImGui::DragFloat("steer_angle_max_speed_deg", &pfxBike->motorbikeSteeringResourceCachePtr.data->land_steering.steer_angle_max_speed_deg);
        ImGui::DragFloat("steer_curve_falloff", &pfxBike->motorbikeSteeringResourceCachePtr.data->land_steering.steer_curve_falloff);
        ImGui::DragFloat("countersteer_speed_factor", &pfxBike->motorbikeSteeringResourceCachePtr.data->land_steering.countersteer_speed_factor);
        ImGui::DragFloat("steer_in_speed_factor", &pfxBike->motorbikeSteeringResourceCachePtr.data->land_steering.steer_in_speed_factor);
        ImGui::DragFloat("steer_input_power_pc", &pfxBike->motorbikeSteeringResourceCachePtr.data->land_steering.steer_input_power_pc);
        ImGui::DragFloat("steer_input_power_durango", &pfxBike->motorbikeSteeringResourceCachePtr.data->land_steering.steer_input_power_durango);
        ImGui::DragFloat("steer_input_power_orbis", &pfxBike->motorbikeSteeringResourceCachePtr.data->land_steering.steer_input_power_orbis);
        ImGui::DragFloat("wheel_drift_aligning_strength", &pfxBike->motorbikeSteeringResourceCachePtr.data->land_steering.wheel_drift_aligning_strength);

        ImGui::Text("Wheelie");
            ImGui::TreePush("Steering Wheelie");
                ImGui::DragFloat("max_lean_angle_deg", &pfxBike->motorbikeSteeringResourceCachePtr.data->wheelie.max_lean_angle_deg);
                ImGui::DragFloat("input_reactiveness", &pfxBike->motorbikeSteeringResourceCachePtr.data->wheelie.input_reactiveness);
                ImGui::DragFloat("dead_zone", &pfxBike->motorbikeSteeringResourceCachePtr.data->wheelie.dead_zone);
                ImGui::DragFloat("min_speed", &pfxBike->motorbikeSteeringResourceCachePtr.data->wheelie.min_speed);
                ImGui::DragFloat("wheelie_angle_deg", &pfxBike->motorbikeSteeringResourceCachePtr.data->wheelie.wheelie_angle_deg);
                ImGui::DragFloat("wheelie_torque", &pfxBike->motorbikeSteeringResourceCachePtr.data->wheelie.wheelie_torque);
                ImGui::DragFloat("wheelie_center_of_mass_offset x", &pfxBike->motorbikeSteeringResourceCachePtr.data->wheelie.wheelie_center_of_mass_offset[0]);
                ImGui::DragFloat("wheelie_center_of_mass_offset y", &pfxBike->motorbikeSteeringResourceCachePtr.data->wheelie.wheelie_center_of_mass_offset[1]);
                ImGui::DragFloat("wheelie_center_of_mass_offset z", &pfxBike->motorbikeSteeringResourceCachePtr.data->wheelie.wheelie_center_of_mass_offset[2]);
                ImGui::DragFloat("nosie_angle_deg", &pfxBike->motorbikeSteeringResourceCachePtr.data->wheelie.nosie_angle_deg);
                ImGui::DragFloat("nosie_torque", &pfxBike->motorbikeSteeringResourceCachePtr.data->wheelie.nosie_torque);
                ImGui::DragFloat("nosie_center_of_mass_offset x", &pfxBike->motorbikeSteeringResourceCachePtr.data->wheelie.nosie_center_of_mass_offset[0]);
                ImGui::DragFloat("nosie_center_of_mass_offset y", &pfxBike->motorbikeSteeringResourceCachePtr.data->wheelie.nosie_center_of_mass_offset[1]);
                ImGui::DragFloat("nosie_center_of_mass_offset z", &pfxBike->motorbikeSteeringResourceCachePtr.data->wheelie.nosie_center_of_mass_offset[2]);
            ImGui::TreePop();
        ImGui::TreePop();

        pfxBike->ApplyMotorbikeSteering(*pfxBike->motorbikeSteeringResourceCachePtr.data);
    }

    util::hooking::func_call<void>(0x1434A64B0, pfxBike);
}