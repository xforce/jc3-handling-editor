#include "../deps/imgui/imgui.h"

#include <jc3/entities/character.h>
#include <jc3/entities/vehicle.h>

#include <json.hpp>

#include <jc3/hashes/vehicles.h>

#include <jc3/entities/pfx/land_steering.h>

struct JCString
{
    union _Bxty {
        char _Buf[16];
        char *_Ptr;
        char _Alias[16];
    } _Bx;

    unsigned __int64 _Mysize;
    unsigned __int64 _Myres;
    std::allocator<char> _Alval;

    const char* c_str() {
        if (_Myres >= 0x10) {
            return _Bx._Ptr;
        }
        return _Bx._Buf;
    }
};

#include <json.hpp>

nlohmann::json CarSettingsToJson(boost::shared_ptr<jc3::CVehicle> vehicle) {
    auto pfxVehicle = vehicle->PfxVehicle;
    assert(pfxVehicle->GetType() == jc3::PfxType::Car && "This vehicle is not a car");
    auto pfxCar = static_cast<jc3::CPfxCar*>(pfxVehicle);
    
    
    namespace json = nlohmann;
    json::json settings_json;

    settings_json["topSpeed"] = *(float*)((char*)pfxVehicle + 0x3EC);
    settings_json["dragCoefficient"] = vehicle->dragCoefficient;
    settings_json["mass"] = vehicle->mass;
    settings_json["linearDamping"] = vehicle->linearDamping;
    settings_json["angularDamping"] = vehicle->angularDamping;
    settings_json["gravityFactor"] = vehicle->gravityFactor;

    auto engine = pfxCar->landVehicleEngine;
    auto engineTransmission = pfxCar->landVehicleTransmission;
    // TODO(alexander): Find the per vehicle thing for steering
    auto landSteering = util::hooking::func_call<jc3::SLandSteering*>(0x1434CD7E0, &pfxCar->landSteeringResourceCachePtr);
    std::vector<float> wheel_torque_ratio;
    for (int i = 0; i < engineTransmission->transmissionProperties.wheelsTorqueRatio.size; ++i) {
        wheel_torque_ratio.emplace_back(engineTransmission->transmissionProperties.wheelsTorqueRatio.Data[i]);
    }

    std::vector<json::json> wheels;
    std::vector<json::json> wheel_suspension;
    for (int i = 0; i < pfxCar->wheelInfo.size; ++i) {
        auto wheelInfo = &pfxCar->wheelInfo.Data[i];
        json::json wheel_info = {
            { "spinVelocity", wheelInfo->spinVelocity },
            { "spinAngle", wheelInfo->spinAngle },
            { "sideForce", wheelInfo->sideForce },
            { "forwardSlipVelocity", wheelInfo->forwardSlipVelocity },
            { "sideSlipVelocity", wheelInfo->sideSlipVelocity },
            { "torque", wheelInfo->torque },
            { "angularVelocity", wheelInfo->angularVelocity },
            { "invInertia", wheelInfo->invInertia },
            { "slipAngleDeg", wheelInfo->slipAngleDeg },
            { "slipRatioSAE", wheelInfo->slipRatioSAE },
            { "camberAngleDeg", wheelInfo->camberAngleDeg },
            { "lateralTireForceOffset", wheelInfo->lateralTireForceOffset },
            { "longitudinalTireForceOffset", wheelInfo->longitudinalTireForceOffset },
            { "tireDragForceOffset", wheelInfo->tireDragForceOffset },
            { "wheelFrictionMultiplier", wheelInfo->wheelFrictionMultiplier },
            { "wheelDragMultiplier", wheelInfo->wheelDragMultiplier },
            { "burnoutFrictionMultiplier", wheelInfo->burnoutFrictionMultiplier },
            { "groundFrictionTorque", wheelInfo->groundFrictionTorque },
            { "unkown1", wheelInfo->unkown1 },
            { "isConstrainedToGround", wheelInfo->isConstrainedToGround },
        };
        wheels.push_back(wheel_info);
    }

    for (int i = 0; i < pfxCar->wheelInfo.size; ++i) {
        auto &suspension = pfxCar->wheelSuspensionConstants[i];
        auto properties = suspension.suspensionProperties;
        json::json meow_suspension = {
            { "suspensionForceMagnitudeAtRest_N", suspension.suspensionForceMagnitudeAtRest_N },
            { "suspensionLengthAtRest_m", suspension.suspensionLengthAtRest_m },
            { "properties",{
                { "antirollbar_strength", properties->antirollbar_strength },
                { "compression", properties->compression },
                { "length", properties->length },
                { "relaxation", properties->relaxation },
                { "strength", properties->strength },
                { "lateral_tire_force_offset", properties->lateral_tire_force_offset },
                { "longitudinal_tire_force_offset", properties->longitudinal_tire_force_offset },
                { "tire_drag_force_offset", properties->tire_drag_force_offset },
                { "hardpoint_offset_along_spring", properties->hardpoint_offset_along_spring },
            } },
        };
        wheel_suspension.push_back(meow_suspension);
    }

    std::vector<float> gear_ratios;
    std::vector<float> upshift_rpm;
    std::vector<float> downshift_rpm;
    for (auto &gear_ratio : pfxCar->transmissionResourceCachePtr.data->gear_ratios)
    {
        gear_ratios.push_back(gear_ratio);
    }

    for (auto &upshift_rpm_ : pfxCar->transmissionResourceCachePtr.data->upshift_rpm)
    {
        upshift_rpm.push_back(upshift_rpm_);
    }

    for (auto &downshift_rpm_ : pfxCar->transmissionResourceCachePtr.data->downshift_rpm)
    {
        downshift_rpm.push_back(downshift_rpm_);
    }

    settings_json["car"] = {
        { "top_speed_kph", pfxCar->topSpeedKph },
        { "top_speed", *(float*)((char*)pfxVehicle + 0x3EC) },
        { "drag_coefficient", vehicle->dragCoefficient },
        { "mass", vehicle->mass },
        { "linearDamping", vehicle->linearDamping },
        { "angularDamping", vehicle->angularDamping },
        { "gravityFactor", vehicle->gravityFactor },
        { "land_global",{
            { "linear_damping_x", pfxCar->customLandGloabl->linear_damping[0] },
            { "linear_damping_y", pfxCar->customLandGloabl->linear_damping[1] },
            { "linear_damping_z", pfxCar->customLandGloabl->linear_damping[2] },
            { "gravity_multiplier_grounded", pfxCar->customLandGloabl->gravity_multiplier_grounded },
            { "gravity_multiplier_in_air_up", pfxCar->customLandGloabl->gravity_multiplier_in_air_up },
            { "gravity_multiplier_in_air_down", pfxCar->customLandGloabl->gravity_multiplier_in_air_down },
            { "takeoff_pitch_damping", pfxCar->customLandGloabl->takeoff_pitch_damping },
            { "front_wheels_damage",{
                { "skew_health", pfxCar->customLandGloabl->front_wheels_damage.skew_health },
                { "broken_wheel_friction_fraction", pfxCar->customLandGloabl->front_wheels_damage.broken_wheel_friction_fraction },
                { "broken_wheel_radius_fraction", pfxCar->customLandGloabl->front_wheels_damage.broken_wheel_radius_fraction },
            } },
            { "rear_wheels_damage",{
                { "skew_health", pfxCar->customLandGloabl->rear_wheels_damage.skew_health },
                { "broken_wheel_friction_fraction", pfxCar->customLandGloabl->rear_wheels_damage.broken_wheel_friction_fraction },
                { "broken_wheel_radius_fraction", pfxCar->customLandGloabl->rear_wheels_damage.broken_wheel_radius_fraction },
            } },
            { "drift",{
                { "drift_entry_slip_angle", pfxCar->customLandGloabl->drift.drift_entry_slip_angle },
                { "drift_exit_slip_angle", pfxCar->customLandGloabl->drift.drift_exit_slip_angle },
                { "max_drift_angle_deg", pfxCar->customLandGloabl->drift.max_drift_angle_deg },
                { "drift_limit_spread_angle_deg", pfxCar->customLandGloabl->drift.drift_limit_spread_angle_deg },
                { "constant_drift_torque", pfxCar->customLandGloabl->drift.constant_drift_torque },
                { "max_drift_torque", pfxCar->customLandGloabl->drift.max_drift_torque },
                { "counter_steer_torque", pfxCar->customLandGloabl->drift.counter_steer_torque },
                { "counter_steer_torque_handbrake", pfxCar->customLandGloabl->drift.counter_steer_torque_handbrake },
                { "counter_steer_torque_brake", pfxCar->customLandGloabl->drift.counter_steer_torque_brake },
                { "drift_yaw_vel_damp", pfxCar->customLandGloabl->drift.drift_yaw_vel_damp },
                { "overdrift_yaw_vel_damp", pfxCar->customLandGloabl->drift.overdrift_yaw_vel_damp },
                { "exit_drift_yaw_vel_damp", pfxCar->customLandGloabl->drift.exit_drift_yaw_vel_damp },
                { "velocity_rotation_start_angle", pfxCar->customLandGloabl->drift.velocity_rotation_start_angle },
                { "velocity_rotation_end_angle", pfxCar->customLandGloabl->drift.velocity_rotation_end_angle },
                { "velocity_rotation_amount", pfxCar->customLandGloabl->drift.velocity_rotation_amount },
                { "velocity_rotation_angle_exp", pfxCar->customLandGloabl->drift.velocity_rotation_angle_exp },
                { "counter_steer_rot_factor", pfxCar->customLandGloabl->drift.counter_steer_rot_factor },
                { "steering_sensitivity", pfxCar->customLandGloabl->drift.steering_sensitivity },
                { "min_speed_to_drift_kmph", pfxCar->customLandGloabl->drift.min_speed_to_drift_kmph },
                { "keep_velocity_strength", pfxCar->customLandGloabl->drift.keep_velocity_strength },
                { "max_keep_velocity_acceleration_g", pfxCar->customLandGloabl->drift.max_keep_velocity_acceleration_g },
            } },
            { "arcade",{
                { "heat_boost",{
                    { "torque_multiplier", pfxCar->customLandGloabl->arcade.heat_boost.torque_multiplier },
                    { "grip_multiplier", pfxCar->customLandGloabl->arcade.heat_boost.grip_multiplier },
                    { "push_force", pfxCar->customLandGloabl->arcade.heat_boost.push_force },
                    { "boost_blend_time", pfxCar->customLandGloabl->arcade.heat_boost.boost_blend_time },
                    { "extra_top_speed", pfxCar->customLandGloabl->arcade.heat_boost.extra_top_speed },
                } },
                { "nitro_boost",{
                    { "torque_multiplier", pfxCar->customLandGloabl->arcade.nitro_boost.torque_multiplier },
                    { "grip_multiplier", pfxCar->customLandGloabl->arcade.nitro_boost.grip_multiplier },
                    { "push_force", pfxCar->customLandGloabl->arcade.nitro_boost.push_force },
                    { "boost_blend_time", pfxCar->customLandGloabl->arcade.nitro_boost.boost_blend_time },
                    { "extra_top_speed", pfxCar->customLandGloabl->arcade.nitro_boost.extra_top_speed },
                } },
                { "nitro_boost_upgraded",{
                    { "torque_multiplier", pfxCar->customLandGloabl->arcade.nitro_boost_upgraded.torque_multiplier },
                    { "grip_multiplier", pfxCar->customLandGloabl->arcade.nitro_boost_upgraded.grip_multiplier },
                    { "push_force", pfxCar->customLandGloabl->arcade.nitro_boost_upgraded.push_force },
                    { "boost_blend_time", pfxCar->customLandGloabl->arcade.nitro_boost_upgraded.boost_blend_time },
                    { "extra_top_speed", pfxCar->customLandGloabl->arcade.nitro_boost_upgraded.extra_top_speed },
                } },
                { "turbo_jump",{
                    { "f_multiplier", pfxCar->customLandGloabl->arcade.turbo_jump.f_multiplier },
                    { "r_multiplier", pfxCar->customLandGloabl->arcade.turbo_jump.r_multiplier },
                    { "punch_delay_time", pfxCar->customLandGloabl->arcade.turbo_jump.punch_delay_time },
                    { "punch_speed_kph", pfxCar->customLandGloabl->arcade.turbo_jump.punch_speed_kph },
                    { "top_speed_kph", pfxCar->customLandGloabl->arcade.turbo_jump.top_speed_kph },
                    { "top_speed_jump_multiplier", pfxCar->customLandGloabl->arcade.turbo_jump.top_speed_jump_multiplier },
                } },
                { "turbo_jump_upgraded",{
                    { "f_multiplier", pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.f_multiplier },
                    { "r_multiplier", pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.r_multiplier },
                    { "punch_delay_time", pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.punch_delay_time },
                    { "punch_speed_kph", pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.punch_speed_kph },
                    { "top_speed_kph", pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.top_speed_kph },
                    { "top_speed_jump_multiplier", pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.top_speed_jump_multiplier },
                } },
            } },
        } },
        { "engine",{
            { "isClutching", *(bool*)&engine->isClutching },
            { "clutchDelay", engine->clutchDelay },
            { "clutchingTime", engine->clutchingTime },
            { "clutchAmount", engine->clutchAmount },
            { "manualClutchEngageTimer", engine->manualClutchEngageTimer },
            { "sourceClutchRpm", engine->sourceClutchRpm },
            { "targetClutchRpm", engine->targetClutchRpm },
            { "engineRevs", engine->engineRevs },
            { "engineDamage", engine->engineDamage },
            { "revLimiterMagnitudeRPM", engine->revLimiterMagnitudeRPM },
            { "isRevLimiting", *(bool*)&engine->isRevLimiting },
            { "fullLoadTorque", engine->fullLoadTorque },
            { "lowestMaxTorque", engine->lowestMaxTorque },
            { "engineMinNoise", engine->engineMinNoise },
            { "engineDamageNoiseScale", engine->engineDamageNoiseScale },
            { "engineMaxDamageTorqueFactor", engine->engineMaxDamageTorqueFactor },
            { "minRPM", engine->minRPM },
            { "optRPM", engine->optRPM },
            { "maxTorque", engine->maxTorque },
            { "torqueFactorAtMinRPM", engine->torqueFactorAtMinRPM },
            { "torqueFactorAtMaxRPM", engine->torqueFactorAtMaxRPM },
            { "resistanceFactorAtMinRPM", engine->resistanceFactorAtMinRPM },
            { "resistanceFactorAtOptRPM", engine->resistanceFactorAtOptRPM },
            { "resistanceFactorAtMaxRPM", engine->resistanceFactorAtMaxRPM },
            { "clutchSlipRPM", engine->clutchSlipRPM },
            { "maxRPM", engine->maxRPM },
            { "overdriveMaxRPM", engine->overdriveMaxRPM },
            { "isOverdriveActive", engine->isOverdriveActive },
        } },
        { "engine_transmission",{
            { "gears", pfxCar->transmissionResourceCachePtr.data->gears },
            { "gear_ratios", gear_ratios },
            { "upshift_rpm", upshift_rpm },
            { "downshift_rpm", downshift_rpm },
            { "nitrous_gears", pfxCar->transmissionResourceCachePtr.data->nitrous_gears },
            { "sequential", pfxCar->transmissionResourceCachePtr.data->sequential },
            { "manual_clutch", pfxCar->transmissionResourceCachePtr.data->manual_clutch },
            { "manual_clutch_blend_rpm", pfxCar->transmissionResourceCachePtr.data->manual_clutch_blend_rpm },
            { "manual_clutch_blend_time", pfxCar->transmissionResourceCachePtr.data->manual_clutch_blend_time },
            { "forward_ratio_percentage", pfxCar->transmissionResourceCachePtr.data->forward_ratio_percentage },
            { "low_gear_forward_ratio_pct", pfxCar->transmissionResourceCachePtr.data->low_gear_forward_ratio_pct },
            { "top_speed", pfxCar->transmissionResourceCachePtr.data->top_speed },
            { "low_gears_final_drive", pfxCar->transmissionResourceCachePtr.data->low_gears_final_drive },
            { "final_drive", pfxCar->transmissionResourceCachePtr.data->final_drive },
            { "reverse_uses_forward_gears", pfxCar->transmissionResourceCachePtr.data->reverse_uses_forward_gears },
            { "reverse_gear_ratio", pfxCar->transmissionResourceCachePtr.data->reverse_gear_ratio },
            { "clutch_delay", pfxCar->transmissionResourceCachePtr.data->clutch_delay },
            { "decay_time_to_cruise_rpm", pfxCar->transmissionResourceCachePtr.data->decay_time_to_cruise_rpm },
            { "target_cruise_rpm", pfxCar->transmissionResourceCachePtr.data->target_cruise_rpm },
            { "wheel_torque_ratio", wheel_torque_ratio }
        } },
        { "suspension", wheel_suspension },
        { "brakes",{
            { "front",{
                { "handbrake", pfxCar->brakesResourceCachePtr.data->front.handbrake },
                { "max_brake_torque", pfxCar->brakesResourceCachePtr.data->front.max_brake_torque },
                { "min_time_to_block", pfxCar->brakesResourceCachePtr.data->front.min_time_to_block },
            } },
            { "rear",{
                { "handbrake", pfxCar->brakesResourceCachePtr.data->rear.handbrake },
                { "max_brake_torque", pfxCar->brakesResourceCachePtr.data->rear.max_brake_torque },
                { "min_time_to_block", pfxCar->brakesResourceCachePtr.data->rear.min_time_to_block },
            } },
            { "handbrake_friction_factor", pfxCar->brakesResourceCachePtr.data->handbrake_friction_factor },
        } },
        { "aerodynamics",{
            { "air_density", pfxCar->landAerodynamicsResourceCachePtr.data->air_density },
            { "frontal_area", pfxCar->landAerodynamicsResourceCachePtr.data->frontal_area },
            { "drag_coefficient", pfxCar->landAerodynamicsResourceCachePtr.data->drag_coefficient },
            { "top_speed_drag_coefficient", pfxCar->landAerodynamicsResourceCachePtr.data->top_speed_drag_coefficient },
            { "lift_coefficient", pfxCar->landAerodynamicsResourceCachePtr.data->lift_coefficient },
        } },
        { "wheels", wheels }
    };

    if (landSteering) {
        settings_json["car"]["steering"] = {
            { "dead_zone", landSteering->dead_zone },
            { "saturation_zone", landSteering->saturation_zone },
            { "t_to_full_steer_s", landSteering->t_to_full_steer_s },
            { "max_speed_t_to_full_steer_s", landSteering->max_speed_t_to_full_steer_s },
            { "min_speed_kmph", landSteering->min_speed_kmph },
            { "max_speed_kmph", landSteering->max_speed_kmph },
            { "steer_angle_min_speed_deg", landSteering->steer_angle_min_speed_deg },
            { "steer_angle_max_speed_deg", landSteering->steer_angle_max_speed_deg },
            { "steer_curve_falloff", landSteering->steer_curve_falloff },
            { "countersteer_speed_factor", landSteering->countersteer_speed_factor },
            { "steer_in_speed_factor", landSteering->steer_in_speed_factor },
            { "steer_input_power_pc", landSteering->steer_input_power_pc },
            { "steer_input_power_durango", landSteering->steer_input_power_durango },
            { "steer_input_power_orbis", landSteering->steer_input_power_orbis },
            { "wheel_drift_aligning_strength", landSteering->wheel_drift_aligning_strength },
        };
    }
    return settings_json;
}

void CarSettingsFromJson(boost::shared_ptr<jc3::CVehicle> vehicle, nlohmann::json settings_json) {
    auto pfxVehicle = vehicle->PfxVehicle;
    assert(pfxVehicle->GetType() == jc3::PfxType::Car && "This vehicle is not a car");
    auto pfxCar = static_cast<jc3::CPfxCar*>(pfxVehicle);

    *(float*)((char*)pfxVehicle + 0x3EC) = settings_json.value("topSpeed", *(float*)((char*)pfxVehicle + 0x3EC));
    vehicle->dragCoefficient = settings_json.value("dragCoefficient", vehicle->dragCoefficient);
    vehicle->mass = settings_json.value("mass", vehicle->mass);
    vehicle->linearDamping = settings_json.value("linearDamping", vehicle->linearDamping);
    vehicle->angularDamping = settings_json.value("angularDamping", vehicle->angularDamping);
    vehicle->gravityFactor = settings_json.value("gravityFactor", vehicle->gravityFactor);

    //if (settings_json.find("gravityModifiers") != settings_json.end()) {
    //	pfxCar->someGravityModifiers->gravityMultiplierGrounded = settings_json["gravityModifiers"].value("gravityGrounded", pfxCar->someGravityModifiers->gravityMultiplierGrounded);
    //}

    if (settings_json.find("car") == settings_json.end()) {
        return;
    }

    auto car_json = settings_json["car"];

    pfxCar->topSpeedKph = settings_json["car"]["top_speed_kph"];
    *(float*)((char*)pfxVehicle + 0x3EC) = settings_json["car"]["top_speed"];
    vehicle->dragCoefficient = car_json["drag_coefficient"];
    vehicle->mass = car_json["mass"];
    vehicle->linearDamping = car_json["linearDamping"];
    vehicle->angularDamping = car_json["angularDamping"];
    vehicle->gravityFactor = car_json["gravityFactor"];

    if(car_json.find("land_global") != car_json.end()) {
        auto global_json = car_json["land_global"];
        pfxCar->customLandGloabl->linear_damping[0] = global_json.value("linear_damping_x", pfxCar->customLandGloabl->linear_damping[0]);
        pfxCar->customLandGloabl->linear_damping[1] = global_json.value("linear_damping_y", pfxCar->customLandGloabl->linear_damping[1]);
        pfxCar->customLandGloabl->linear_damping[2] = global_json.value("linear_damping_z", pfxCar->customLandGloabl->linear_damping[2]);
        pfxCar->customLandGloabl->linear_damping[2] = global_json.value("linear_damping_z", pfxCar->customLandGloabl->linear_damping[2]);
        pfxCar->customLandGloabl->gravity_multiplier_grounded = global_json.value("gravity_multiplier_grounded", pfxCar->customLandGloabl->gravity_multiplier_grounded);
        pfxCar->customLandGloabl->gravity_multiplier_in_air_up = global_json.value("gravity_multiplier_in_air_up", pfxCar->customLandGloabl->gravity_multiplier_in_air_up);
        pfxCar->customLandGloabl->gravity_multiplier_in_air_down = global_json.value("gravity_multiplier_in_air_down", pfxCar->customLandGloabl->gravity_multiplier_in_air_down);
        pfxCar->customLandGloabl->takeoff_pitch_damping = global_json.value("takeoff_pitch_damping", pfxCar->customLandGloabl->takeoff_pitch_damping);

        if (global_json.find("front_wheels_damage") != global_json.end()) {
            pfxCar->customLandGloabl->front_wheels_damage.skew_health = global_json["front_wheels_damage"].value("skew_health", pfxCar->customLandGloabl->front_wheels_damage.skew_health);
            pfxCar->customLandGloabl->front_wheels_damage.broken_wheel_friction_fraction = global_json["front_wheels_damage"].value("broken_wheel_friction_fraction", pfxCar->customLandGloabl->front_wheels_damage.broken_wheel_friction_fraction);
            pfxCar->customLandGloabl->front_wheels_damage.broken_wheel_radius_fraction = global_json["front_wheels_damage"].value("broken_wheel_radius_fraction", pfxCar->customLandGloabl->front_wheels_damage.broken_wheel_radius_fraction);
        }

        if (global_json.find("rear_wheels_damage") != global_json.end()) {
            pfxCar->customLandGloabl->rear_wheels_damage.skew_health = global_json["rear_wheels_damage"].value("skew_health", pfxCar->customLandGloabl->front_wheels_damage.skew_health);
            pfxCar->customLandGloabl->rear_wheels_damage.broken_wheel_friction_fraction = global_json["rear_wheels_damage"].value("broken_wheel_friction_fraction", pfxCar->customLandGloabl->front_wheels_damage.broken_wheel_friction_fraction);
            pfxCar->customLandGloabl->rear_wheels_damage.broken_wheel_radius_fraction = global_json["rear_wheels_damage"].value("broken_wheel_radius_fraction", pfxCar->customLandGloabl->front_wheels_damage.broken_wheel_radius_fraction);
        }

        if (global_json.find("drift") != global_json.end()) {
            auto drift_json = global_json["drift"];
            pfxCar->customLandGloabl->drift.drift_entry_slip_angle = drift_json.value("drift_entry_slip_angle", pfxCar->customLandGloabl->drift.drift_entry_slip_angle);
            pfxCar->customLandGloabl->drift.drift_exit_slip_angle = drift_json.value("drift_exit_slip_angle", pfxCar->customLandGloabl->drift.drift_exit_slip_angle);
            pfxCar->customLandGloabl->drift.max_drift_angle_deg = drift_json.value("max_drift_angle_deg", pfxCar->customLandGloabl->drift.max_drift_angle_deg);
            pfxCar->customLandGloabl->drift.drift_limit_spread_angle_deg = drift_json.value("drift_limit_spread_angle_deg", pfxCar->customLandGloabl->drift.drift_limit_spread_angle_deg);
            pfxCar->customLandGloabl->drift.constant_drift_torque = drift_json.value("constant_drift_torque", pfxCar->customLandGloabl->drift.constant_drift_torque);
            pfxCar->customLandGloabl->drift.max_drift_torque = drift_json.value("max_drift_torque", pfxCar->customLandGloabl->drift.max_drift_torque);
            pfxCar->customLandGloabl->drift.counter_steer_torque = drift_json.value("drift_entry_slip_angle", pfxCar->customLandGloabl->drift.counter_steer_torque);
            pfxCar->customLandGloabl->drift.counter_steer_torque_handbrake = drift_json.value("counter_steer_torque_handbrake", pfxCar->customLandGloabl->drift.counter_steer_torque_handbrake);
            pfxCar->customLandGloabl->drift.counter_steer_torque_brake = drift_json.value("counter_steer_torque_brake", pfxCar->customLandGloabl->drift.counter_steer_torque_brake);
            pfxCar->customLandGloabl->drift.drift_yaw_vel_damp = drift_json.value("drift_yaw_vel_damp", pfxCar->customLandGloabl->drift.drift_yaw_vel_damp);
            pfxCar->customLandGloabl->drift.overdrift_yaw_vel_damp = drift_json.value("overdrift_yaw_vel_damp", pfxCar->customLandGloabl->drift.overdrift_yaw_vel_damp);
            pfxCar->customLandGloabl->drift.exit_drift_yaw_vel_damp = drift_json.value("exit_drift_yaw_vel_damp", pfxCar->customLandGloabl->drift.exit_drift_yaw_vel_damp);
            pfxCar->customLandGloabl->drift.velocity_rotation_start_angle = drift_json.value("velocity_rotation_start_angle", pfxCar->customLandGloabl->drift.velocity_rotation_start_angle);
            pfxCar->customLandGloabl->drift.velocity_rotation_end_angle = drift_json.value("velocity_rotation_end_angle", pfxCar->customLandGloabl->drift.velocity_rotation_end_angle);
            pfxCar->customLandGloabl->drift.velocity_rotation_amount = drift_json.value("velocity_rotation_amount", pfxCar->customLandGloabl->drift.velocity_rotation_amount);
            pfxCar->customLandGloabl->drift.velocity_rotation_angle_exp = drift_json.value("velocity_rotation_angle_exp", pfxCar->customLandGloabl->drift.velocity_rotation_angle_exp);
            pfxCar->customLandGloabl->drift.counter_steer_rot_factor = drift_json.value("counter_steer_rot_factor", pfxCar->customLandGloabl->drift.counter_steer_rot_factor);
            pfxCar->customLandGloabl->drift.steering_sensitivity = drift_json.value("steering_sensitivity", pfxCar->customLandGloabl->drift.steering_sensitivity);

            pfxCar->customLandGloabl->drift.min_speed_to_drift_kmph = drift_json.value("min_speed_to_drift_kmph", pfxCar->customLandGloabl->drift.min_speed_to_drift_kmph);
            pfxCar->customLandGloabl->drift.keep_velocity_strength = drift_json.value("keep_velocity_strength", pfxCar->customLandGloabl->drift.keep_velocity_strength);
            pfxCar->customLandGloabl->drift.max_keep_velocity_acceleration_g = drift_json.value("max_keep_velocity_acceleration_g", pfxCar->customLandGloabl->drift.max_keep_velocity_acceleration_g);
        }

        if (global_json.find("arcade") != global_json.end()) {
            auto arcade_json = global_json["arcade"];
            if (arcade_json.find("heat_boost") != arcade_json.end()) {
                pfxCar->customLandGloabl->arcade.heat_boost.torque_multiplier = arcade_json["heat_boost"].value("torque_multiplier", pfxCar->customLandGloabl->arcade.heat_boost.torque_multiplier);
                pfxCar->customLandGloabl->arcade.heat_boost.grip_multiplier = arcade_json["heat_boost"].value("grip_multiplier", pfxCar->customLandGloabl->arcade.heat_boost.grip_multiplier);
                pfxCar->customLandGloabl->arcade.heat_boost.push_force = arcade_json["heat_boost"].value("push_force", pfxCar->customLandGloabl->arcade.heat_boost.push_force);
                pfxCar->customLandGloabl->arcade.heat_boost.boost_blend_time = arcade_json["heat_boost"].value("boost_blend_time", pfxCar->customLandGloabl->arcade.heat_boost.boost_blend_time);
                pfxCar->customLandGloabl->arcade.heat_boost.extra_top_speed = arcade_json["heat_boost"].value("extra_top_speed", pfxCar->customLandGloabl->arcade.heat_boost.extra_top_speed);
            }
            if (arcade_json.find("nitro_boost") != arcade_json.end()) {
                pfxCar->customLandGloabl->arcade.nitro_boost.torque_multiplier = arcade_json["nitro_boost"].value("torque_multiplier", pfxCar->customLandGloabl->arcade.nitro_boost.torque_multiplier);
                pfxCar->customLandGloabl->arcade.nitro_boost.grip_multiplier = arcade_json["nitro_boost"].value("grip_multiplier", pfxCar->customLandGloabl->arcade.nitro_boost.grip_multiplier);
                pfxCar->customLandGloabl->arcade.nitro_boost.push_force = arcade_json["nitro_boost"].value("push_force", pfxCar->customLandGloabl->arcade.nitro_boost.push_force);
                pfxCar->customLandGloabl->arcade.nitro_boost.boost_blend_time = arcade_json["nitro_boost"].value("boost_blend_time", pfxCar->customLandGloabl->arcade.nitro_boost.boost_blend_time);
                pfxCar->customLandGloabl->arcade.nitro_boost.extra_top_speed = arcade_json["nitro_boost"].value("extra_top_speed", pfxCar->customLandGloabl->arcade.nitro_boost.extra_top_speed);
            }
            if (arcade_json.find("nitro_boost_upgraded") != arcade_json.end()) {
                pfxCar->customLandGloabl->arcade.nitro_boost_upgraded.torque_multiplier = arcade_json["nitro_boost_upgraded"].value("torque_multiplier", pfxCar->customLandGloabl->arcade.nitro_boost_upgraded.torque_multiplier);
                pfxCar->customLandGloabl->arcade.nitro_boost_upgraded.grip_multiplier = arcade_json["nitro_boost_upgraded"].value("grip_multiplier", pfxCar->customLandGloabl->arcade.nitro_boost_upgraded.grip_multiplier);
                pfxCar->customLandGloabl->arcade.nitro_boost_upgraded.push_force = arcade_json["nitro_boost_upgraded"].value("push_force", pfxCar->customLandGloabl->arcade.nitro_boost_upgraded.push_force);
                pfxCar->customLandGloabl->arcade.nitro_boost_upgraded.boost_blend_time = arcade_json["nitro_boost_upgraded"].value("boost_blend_time", pfxCar->customLandGloabl->arcade.nitro_boost_upgraded.boost_blend_time);
                pfxCar->customLandGloabl->arcade.nitro_boost_upgraded.extra_top_speed = arcade_json["nitro_boost_upgraded"].value("extra_top_speed", pfxCar->customLandGloabl->arcade.nitro_boost_upgraded.extra_top_speed);
            }
            if (arcade_json.find("turbo_jump") != arcade_json.end()) {
                pfxCar->customLandGloabl->arcade.turbo_jump.f_multiplier = arcade_json["turbo_jump"].value("f_multiplier", pfxCar->customLandGloabl->arcade.turbo_jump.f_multiplier);
                pfxCar->customLandGloabl->arcade.turbo_jump.r_multiplier = arcade_json["turbo_jump"].value("r_multiplier", pfxCar->customLandGloabl->arcade.turbo_jump.r_multiplier);
                pfxCar->customLandGloabl->arcade.turbo_jump.punch_delay_time = arcade_json["turbo_jump"].value("punch_delay_time", pfxCar->customLandGloabl->arcade.turbo_jump.punch_delay_time);
                pfxCar->customLandGloabl->arcade.turbo_jump.punch_speed_kph = arcade_json["turbo_jump"].value("punch_speed_kph", pfxCar->customLandGloabl->arcade.turbo_jump.punch_speed_kph);
                pfxCar->customLandGloabl->arcade.turbo_jump.top_speed_kph = arcade_json["turbo_jump"].value("top_speed_kph", pfxCar->customLandGloabl->arcade.turbo_jump.top_speed_kph);
                pfxCar->customLandGloabl->arcade.turbo_jump.top_speed_jump_multiplier = arcade_json["turbo_jump"].value("f_multiplier", pfxCar->customLandGloabl->arcade.turbo_jump.top_speed_jump_multiplier);
            }
            if (arcade_json.find("turbo_jump_upgraded") != arcade_json.end()) {
                pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.f_multiplier = arcade_json["turbo_jump_upgraded"].value("f_multiplier", pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.f_multiplier);
                pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.r_multiplier = arcade_json["turbo_jump_upgraded"].value("r_multiplier", pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.r_multiplier);
                pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.punch_delay_time = arcade_json["turbo_jump_upgraded"].value("punch_delay_time", pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.punch_delay_time);
                pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.punch_speed_kph = arcade_json["turbo_jump_upgraded"].value("punch_speed_kph", pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.punch_speed_kph);
                pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.top_speed_kph = arcade_json["turbo_jump_upgraded"].value("top_speed_kph", pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.top_speed_kph);
                pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.top_speed_jump_multiplier = arcade_json["turbo_jump"].value("f_multiplier", pfxCar->customLandGloabl->arcade.turbo_jump_upgraded.top_speed_jump_multiplier);
            }
        }
    }

    auto engine = pfxCar->landVehicleEngine;
    if (settings_json["car"].find("engine") != settings_json.end()) {
        auto & engine_json = settings_json["car"]["engine"];
        engine->isClutching = engine_json.value("isClutching", *(bool*)&engine->isClutching);
        engine->clutchDelay = engine_json.value("clutchDelay", engine->clutchDelay);
        engine->clutchingTime = engine_json.value( "clutchingTime", engine->clutchingTime );
        engine->clutchAmount = engine_json.value( "clutchAmount", engine->clutchAmount );
        engine->manualClutchEngageTimer = engine_json.value( "manualClutchEngageTimer", engine->manualClutchEngageTimer );
        engine->sourceClutchRpm = engine_json.value( "sourceClutchRpm", engine->sourceClutchRpm );
        engine->targetClutchRpm = engine_json.value( "targetClutchRpm", engine->targetClutchRpm );
        engine->engineRevs = engine_json.value( "engineRevs", engine->engineRevs );
        engine->engineDamage = engine_json.value( "engineDamage", engine->engineDamage );
        engine->revLimiterMagnitudeRPM = engine_json.value( "revLimiterMagnitudeRPM", engine->revLimiterMagnitudeRPM );
        engine->isRevLimiting = engine_json.value( "isRevLimiting", *(bool*)&engine->isRevLimiting );
        engine->fullLoadTorque = engine_json.value( "fullLoadTorque", engine->fullLoadTorque );
        engine->lowestMaxTorque = engine_json.value( "lowestMaxTorque", engine->lowestMaxTorque );
        engine->engineMinNoise = engine_json.value( "engineMinNoise", engine->engineMinNoise );
        engine->engineDamageNoiseScale = engine_json.value( "engineDamageNoiseScale", engine->engineDamageNoiseScale );
        engine->engineMaxDamageTorqueFactor = engine_json.value( "engineMaxDamageTorqueFactor", engine->engineMaxDamageTorqueFactor );
        engine->minRPM = engine_json.value( "minRPM", engine->minRPM);
        engine->optRPM = engine_json.value( "optRPM", engine->optRPM);
        engine->maxTorque = engine_json.value( "maxTorque", engine->maxTorque);
        engine->torqueFactorAtMinRPM = engine_json.value( "torqueFactorAtMinRPM", engine->torqueFactorAtMinRPM);
        engine->torqueFactorAtMaxRPM = engine_json.value( "torqueFactorAtMaxRPM", engine->torqueFactorAtMaxRPM);
        engine->resistanceFactorAtMinRPM = engine_json.value( "resistanceFactorAtMinRPM", engine->resistanceFactorAtMinRPM);
        engine->resistanceFactorAtOptRPM = engine_json.value( "resistanceFactorAtOptRPM", engine->resistanceFactorAtOptRPM);
        engine->resistanceFactorAtMaxRPM = engine_json.value( "resistanceFactorAtMaxRPM", engine->resistanceFactorAtMaxRPM);
        engine->clutchSlipRPM = engine_json.value( "clutchSlipRPM", engine->clutchSlipRPM);
        engine->maxRPM = engine_json.value( "maxRPM", engine->maxRPM);
        engine->overdriveMaxRPM = engine_json.value( "overdriveMaxRPM", engine->overdriveMaxRPM);
        engine->isOverdriveActive = engine_json.value("isOverdriveActive", engine->isOverdriveActive);
    }

    std::vector<jc3::WheelInfo> wheel_infos;
    for (auto & meow : settings_json["car"]["wheels"]) {
        jc3::WheelInfo wheel_info;
        wheel_info.spinVelocity = meow["spinVelocity"];
        wheel_info.spinAngle = meow["spinAngle"];
        wheel_info.sideForce = meow["sideForce"];
        wheel_info.forwardSlipVelocity = meow["forwardSlipVelocity"];
        wheel_info.sideSlipVelocity = meow["sideSlipVelocity"];
        wheel_info.torque = meow["torque"];
        wheel_info.angularVelocity = meow["angularVelocity"];
        wheel_info.invInertia = meow["invInertia"];
        wheel_info.slipAngleDeg = meow["slipAngleDeg"];
        wheel_info.slipRatioSAE = meow["slipRatioSAE"];
        wheel_info.camberAngleDeg = meow["camberAngleDeg"];
        wheel_info.lateralTireForceOffset = meow["lateralTireForceOffset"];
        wheel_info.longitudinalTireForceOffset = meow["longitudinalTireForceOffset"];
        wheel_info.tireDragForceOffset = meow["tireDragForceOffset"];
        wheel_info.wheelFrictionMultiplier = meow["wheelFrictionMultiplier"];

        wheel_info.wheelDragMultiplier = meow["wheelDragMultiplier"];
        wheel_info.burnoutFrictionMultiplier = meow["burnoutFrictionMultiplier"];
        wheel_info.groundFrictionTorque = meow["groundFrictionTorque"];
        wheel_info.unkown1 = meow["unkown1"];
        wheel_info.isConstrainedToGround = meow["isConstrainedToGround"];
        wheel_infos.emplace_back(wheel_info);
    }

    try {
        for (int i = 0; i < pfxCar->wheelInfo.size; ++i) {
            pfxCar->wheelInfo.Data[i] = wheel_infos.at(i);
        }
    }
    catch (...) {}

    std::vector<jc3::SWheelSuspensionConstant> wheel_suspension;
    if (car_json.find("suspension") != car_json.end()) {
        int i = 0;
        for (auto & meow : car_json["suspension"]) {
            auto &suspension = pfxCar->wheelSuspensionConstants[i];
            auto properties = suspension.suspensionProperties;
            suspension.suspensionForceMagnitudeAtRest_N = meow.value("suspensionForceMagnitudeAtRest_N", suspension.suspensionForceMagnitudeAtRest_N);
            suspension.suspensionLengthAtRest_m = meow.value("suspensionLengthAtRest_m", suspension.suspensionLengthAtRest_m);
            properties->antirollbar_strength = meow.value("antirollbar_strength", properties->antirollbar_strength);
            properties->compression = meow.value("compression", properties->compression);
            properties->length = meow.value("length", properties->length);
            properties->relaxation = meow.value("relaxation", properties->relaxation);
            properties->strength = meow.value("strength", properties->strength);
            properties->lateral_tire_force_offset = meow.value("lateral_tire_force_offset", properties->lateral_tire_force_offset);
            properties->longitudinal_tire_force_offset = meow.value("longitudinal_tire_force_offset", properties->longitudinal_tire_force_offset);
            properties->tire_drag_force_offset = meow.value("tire_drag_force_offset", properties->tire_drag_force_offset);
            properties->hardpoint_offset_along_spring = meow.value("hardpoint_offset_along_spring", properties->hardpoint_offset_along_spring);
            ++i;
            // Let's hope we don't get fucked here...
        }
    }
}

void DoCarHandlingUI(boost::shared_ptr<jc3::CVehicle> real_vehicle, jc3::CPfxVehicle *pfxVehicle) {

    auto pfxCar = static_cast<jc3::CPfxCar*>(pfxVehicle);

    using json = nlohmann::json;
    static json vehicle_hashes = json::parse(jc3::vehicle_hashes);
    assert(vehicle_hashes.is_array() && "Vehicle hashes is not an array");


    // Car stuff
    auto hash = real_vehicle->GetNameHash();

    for (auto &vehicle : vehicle_hashes) {
        if (vehicle["hash"].is_number() && static_cast<uint32_t>(vehicle["hash"]) == hash) {
            std::string t = vehicle["model_name"];
            ImGui::BulletText("Model Name: %s", t.c_str());
        }
    }
    
    ImGui::BulletText("Engine Torque %f", pfxCar->engineTorque);
    ImGui::BulletText("Engine RPM %f", pfxCar->engineRPM);
    ImGui::BulletText("Top Speed %f", pfxCar->topSpeedKph);
    ImGui::Separator();

    ImGui::DragFloat("Top Speed", (float*)((char*)pfxVehicle + 0x3EC));

    ImGui::DragFloat("Drag Coefficient", &real_vehicle->dragCoefficient);
    ImGui::DragFloat("Mass", &real_vehicle->mass);
    ImGui::DragFloat("Linear Damping", &real_vehicle->linearDamping);
    ImGui::DragFloat("Angular Damping", &real_vehicle->angularDamping);
    ImGui::SliderFloat("Gravity Factor", &real_vehicle->gravityFactor, -128, 128);

    ImGui::Separator();

    if (ImGui::CollapsingHeader("Gravity Modifiers")) {
        ImGui::TreePush("Gravity Modifiers");
        //ImGui::DragFloat("Gravity Grounded", &pfxCar->someGravityModifiers->gravityMultiplierGrounded);
        ImGui::TreePop();
    }

    if (ImGui::CollapsingHeader("Engine"))
    {
        ImGui::TreePush("Engine");
        auto engine = pfxCar->landEngineResourceCachePtr.data;
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

        pfxCar->ApplyLandEngine(*pfxCar->landEngineResourceCachePtr.data);
    }

    if (ImGui::CollapsingHeader("Engine Transmission")) {
        ImGui::TreePush("Engine Transmission");

        ImGui::DragInt("gears", &pfxCar->transmissionResourceCachePtr.data->gears);
        ImGui::DragInt("nitrous_gears", &pfxCar->transmissionResourceCachePtr.data->nitrous_gears);

        ImGui::DragInt("sequential", &pfxCar->transmissionResourceCachePtr.data->sequential);
        ImGui::DragInt("manual_clutch", &pfxCar->transmissionResourceCachePtr.data->manual_clutch);

        ImGui::DragFloat("manual_clutch_blend_rpm", &pfxCar->transmissionResourceCachePtr.data->manual_clutch_blend_rpm);
        ImGui::DragFloat("manual_clutch_blend_time", &pfxCar->transmissionResourceCachePtr.data->manual_clutch_blend_time);
        ImGui::DragFloat("forward_ratio_percentage", &pfxCar->transmissionResourceCachePtr.data->forward_ratio_percentage);
        ImGui::DragFloat("low_gear_forward_ratio_pct", &pfxCar->transmissionResourceCachePtr.data->low_gear_forward_ratio_pct);

        ImGui::DragFloat("top_speed", &pfxCar->transmissionResourceCachePtr.data->top_speed);
        ImGui::DragFloat("low_gears_final_drive", &pfxCar->transmissionResourceCachePtr.data->low_gears_final_drive);
        ImGui::DragFloat("final_drive", &pfxCar->transmissionResourceCachePtr.data->final_drive);
        ImGui::DragFloat("reverse_gear_ratio", &pfxCar->transmissionResourceCachePtr.data->reverse_gear_ratio);
        ImGui::DragFloat("clutch_delay", &pfxCar->transmissionResourceCachePtr.data->clutch_delay);
        ImGui::DragFloat("decay_time_to_cruise_rpm", &pfxCar->transmissionResourceCachePtr.data->decay_time_to_cruise_rpm);
        ImGui::DragFloat("target_cruise_rpm", &pfxCar->transmissionResourceCachePtr.data->target_cruise_rpm);

        for (int i = 0; i < pfxCar->transmissionResourceCachePtr.data->gears; ++i) {
            char wheel_text[100];
            sprintf(wheel_text, "Wheel Torque Ratio %d", i);
            ImGui::TreePush(wheel_text);
            ImGui::SliderFloat(wheel_text, &pfxCar->transmissionResourceCachePtr.data->gear_ratios[i], 0, 128);
            ImGui::TreePop();
        }
        ImGui::TreePop();

        pfxCar->ApplyTransmission(*pfxCar->transmissionResourceCachePtr.data);
    }

    if (ImGui::CollapsingHeader("Suspension")) {
        ImGui::TreePush("Suspension");
        for (int i = 0; i < pfxCar->wheelInfo.size; ++i) {
            auto & suspension = pfxCar->wheelSuspensionConstants[i];
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
        ImGui::Checkbox("Handbrake", (bool*)&pfxCar->brakesResourceCachePtr.data->front.handbrake);
        ImGui::DragFloat("Max Brake Torque", &pfxCar->brakesResourceCachePtr.data->front.max_brake_torque);
        ImGui::DragFloat("Time To Block", &pfxCar->brakesResourceCachePtr.data->front.min_time_to_block);
        ImGui::TreePop();
        ImGui::Separator();
        ImGui::Text("Rear");
        ImGui::TreePush("Brakes Rear");
        ImGui::Checkbox("Handbrake", (bool*)&pfxCar->brakesResourceCachePtr.data->rear.handbrake);
        ImGui::DragFloat("Max Brake Torque", &pfxCar->brakesResourceCachePtr.data->rear.max_brake_torque);
        ImGui::DragFloat("Time To Block", &pfxCar->brakesResourceCachePtr.data->rear.min_time_to_block);
        ImGui::TreePop();

        pfxCar->ApplyBrakes(*pfxCar->brakesResourceCachePtr.data);
    }

    if (ImGui::CollapsingHeader("Aerodynamics")) {
        ImGui::TreePush("Aerodynamic");
        ImGui::DragFloat("Air Density", &pfxCar->landAerodynamicsResourceCachePtr.data->air_density);
        ImGui::DragFloat("Frontal Area", &pfxCar->landAerodynamicsResourceCachePtr.data->frontal_area);
        ImGui::DragFloat("Drag Coefficient", &pfxCar->landAerodynamicsResourceCachePtr.data->drag_coefficient);
        ImGui::DragFloat("Top Speed Drag Coefficient", &pfxCar->landAerodynamicsResourceCachePtr.data->top_speed_drag_coefficient);
        ImGui::DragFloat("Lift Coefficient", &pfxCar->landAerodynamicsResourceCachePtr.data->lift_coefficient);
        ImGui::TreePop();

        pfxCar->ApplyLandAerodynamics(*pfxCar->landAerodynamicsResourceCachePtr.data);
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

        for (int i = 0; i < pfxCar->wheelInfo.size; ++i) {
            char wheel_text[100];
            sprintf(wheel_text, "Wheel %d", i);
            ImGui::TreePush(wheel_text);
            if (ImGui::CollapsingHeader(wheel_text)) {
                DrawWheelInfo(wheel_text, &pfxCar->wheelInfo.Data[i]);
            }
            ImGui::TreePop();
        }
    }

    auto landSteering = util::hooking::func_call<jc3::SLandSteering*>(0x1434CD7E0, &pfxCar->landSteeringResourceCachePtr);

    if (landSteering) {
        if (ImGui::CollapsingHeader("Steering")) {
            ImGui::TreePush("Steering");
            ImGui::DragFloat("dead_zone", &landSteering->dead_zone);
            ImGui::DragFloat("saturation_zone", &landSteering->saturation_zone);
            ImGui::DragFloat("t_to_full_steer_s", &landSteering->t_to_full_steer_s);
            ImGui::DragFloat("max_speed_t_to_full_steer_s", &landSteering->max_speed_t_to_full_steer_s);
            ImGui::DragFloat("min_speed_kmph", &landSteering->min_speed_kmph);
            ImGui::DragFloat("max_speed_kmph", &landSteering->max_speed_kmph);
            ImGui::DragFloat("steer_angle_min_speed_deg", &landSteering->steer_angle_min_speed_deg);
            ImGui::DragFloat("steer_angle_max_speed_deg", &landSteering->steer_angle_max_speed_deg);
            ImGui::DragFloat("steer_curve_falloff", &landSteering->steer_curve_falloff);
            ImGui::DragFloat("countersteer_speed_factor", &landSteering->countersteer_speed_factor);
            ImGui::DragFloat("steer_in_speed_factor", &landSteering->steer_in_speed_factor);
            ImGui::DragFloat("steer_input_power_pc", &landSteering->steer_input_power_pc);
            ImGui::DragFloat("steer_input_power_durango", &landSteering->steer_input_power_durango);
            ImGui::DragFloat("steer_input_power_orbis", &landSteering->steer_input_power_orbis);
            ImGui::DragFloat("wheel_drift_aligning_strength", &landSteering->wheel_drift_aligning_strength);
            ImGui::TreePop();

            pfxCar->ApplyLandSteering(*landSteering);
        }
    }

    //util::hooking::func_call<void>(0x143794F60, real_vehicle);
    util::hooking::func_call<void>(0x1434A64B0, pfxCar); // This calculates some speed stuff, don't really know if it is required tbh
}
