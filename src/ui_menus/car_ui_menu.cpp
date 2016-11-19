#include "../deps/imgui/imgui.h"

#include <jc3/entities/character.h>
#include <jc3/entities/vehicle.h>

#include <json.hpp>

#include <jc3/hashes/vehicles.h>

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

struct SProfileItemInfo
{
	char pad[0x18];
	JCString m_UIName;
};

#include <json.hpp>

nlohmann::json CarSettingsToJson(jc3::CVehicle * vehicle) {
	auto pfxVehicle = static_cast<jc3::CVehicle*>(vehicle)->PfxVehicle;
	assert(pfxVehicle->GetType() == jc3::PfxType::Car && "This vehicle is not a car");
	auto pfxCar = static_cast<jc3::CPfxCar*>(pfxVehicle);
	
	
	namespace json = nlohmann;
	json::json settings_json;

	settings_json["topSpeed"] = *(float*)((char*)pfxVehicle + 0x3EC);
	settings_json["dragCoefficient"] = vehicle->DragCoefficient;
	settings_json["mass"] = vehicle->Mass;
	settings_json["linearDamping"] = vehicle->LinearDamping;
	settings_json["angularDamping"] = vehicle->AngularDamping;
	settings_json["gravityFactor"] = vehicle->GravityFactor;

	settings_json["gravityModifiers"] = {
		{ "gravityGrounded", pfxCar->someGravityModifiers->gravityMultiplierGrounded },
	};

	auto engine = pfxCar->landVehicleEngine;
	settings_json["engine"] = {
		{"isClutching", *(bool*)&engine->isClutching },
		{"clutchDelay", engine->clutchDelay },
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
	};

	auto engineTransmission = pfxCar->landVehicleTransmission;
	settings_json["engineTransmission"] = {
		{ "forwardTorqueRatio", engineTransmission->transmissionProperties.forwardTorqueRatio },
		{ "lowGearForwardTorqueRatio", engineTransmission->transmissionProperties.lowGearForwardTorqueRatio },
		{ "maxTransmissionRPM", engineTransmission->transmissionProperties.maxTransmissionRPM },
		{ "maxReversingTransmissionRPM", engineTransmission->transmissionProperties.maxReversingTransmissionRPM },
		{ "targetCruiseRPM", engineTransmission->transmissionProperties.targetCruiseRPM },
		{ "decayTimeToCruiseRPM", engineTransmission->transmissionProperties.decayTimeToCruiseRPM },
		{ "lowGearingPrimaryTransmissionRatio", engineTransmission->transmissionProperties.lowGearingPrimaryTransmissionRatio },
		{ "downshiftRPM", engineTransmission->transmissionProperties.downshiftRPM },
		{ "upshiftRPM", engineTransmission->transmissionProperties.upshiftRPM },
		{ "primaryTransmissionRatio", engineTransmission->transmissionProperties.primaryTransmissionRatio },
	};
	{
		std::vector<float> meow;
		for (int i = 0; i < engineTransmission->transmissionProperties.wheelsTorqueRatio.size; ++i) {
			meow.emplace_back(engineTransmission->transmissionProperties.wheelsTorqueRatio.Data[i]);
		}
		settings_json["engineTransmission"]["wheelTorqueRatio"] = meow;
	}

	return settings_json;
}

void CarSettingsFromJson(jc3::CVehicle * vehicle, nlohmann::json settings_json) {
	auto pfxVehicle = static_cast<jc3::CVehicle*>(vehicle)->PfxVehicle;
	assert(pfxVehicle->GetType() == jc3::PfxType::Car && "This vehicle is not a car");
	auto pfxCar = static_cast<jc3::CPfxCar*>(pfxVehicle);

	*(float*)((char*)pfxVehicle + 0x3EC) = settings_json.value("topSpeed", *(float*)((char*)pfxVehicle + 0x3EC));
	vehicle->DragCoefficient = settings_json.value("dragCoefficient", vehicle->DragCoefficient);
	vehicle->Mass = settings_json.value("mass", vehicle->Mass);
	vehicle->LinearDamping = settings_json.value("linearDamping", vehicle->LinearDamping);
	vehicle->AngularDamping = settings_json.value("angularDamping", vehicle->AngularDamping);
	vehicle->GravityFactor = settings_json.value("gravityFactor", vehicle->GravityFactor);

	if (settings_json.find("gravityModifiers") != settings_json.end()) {
		pfxCar->someGravityModifiers->gravityMultiplierGrounded = settings_json["gravityModifiers"].value("gravityGrounded", pfxCar->someGravityModifiers->gravityMultiplierGrounded);
	}

	auto engine = pfxCar->landVehicleEngine;
	if (settings_json.find("engine") != settings_json.end()) {
		auto & engine_json = settings_json["engine"];
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

	auto engineTransmission = pfxCar->landVehicleTransmission;
	if (settings_json.find("engineTransmission") != settings_json.end()) {
		auto & engineTransmission_json = settings_json["engineTransmission"];
		engineTransmission->transmissionProperties.forwardTorqueRatio = engineTransmission_json.value("forwardTorqueRatio", engineTransmission->transmissionProperties.forwardTorqueRatio);
		engineTransmission->transmissionProperties.lowGearForwardTorqueRatio = engineTransmission_json.value( "lowGearForwardTorqueRatio", engineTransmission->transmissionProperties.lowGearForwardTorqueRatio );
		engineTransmission->transmissionProperties.maxTransmissionRPM = engineTransmission_json.value( "maxTransmissionRPM", engineTransmission->transmissionProperties.maxTransmissionRPM );
		engineTransmission->transmissionProperties.maxReversingTransmissionRPM = engineTransmission_json.value( "maxReversingTransmissionRPM", engineTransmission->transmissionProperties.maxReversingTransmissionRPM );
		engineTransmission->transmissionProperties.targetCruiseRPM = engineTransmission_json.value( "targetCruiseRPM", engineTransmission->transmissionProperties.targetCruiseRPM );
		engineTransmission->transmissionProperties.decayTimeToCruiseRPM = engineTransmission_json.value( "decayTimeToCruiseRPM", engineTransmission->transmissionProperties.decayTimeToCruiseRPM );
		engineTransmission->transmissionProperties.lowGearingPrimaryTransmissionRatio = engineTransmission_json.value( "lowGearingPrimaryTransmissionRatio", engineTransmission->transmissionProperties.lowGearingPrimaryTransmissionRatio );
		engineTransmission->transmissionProperties.downshiftRPM = engineTransmission_json.value( "downshiftRPM", engineTransmission->transmissionProperties.downshiftRPM );
		engineTransmission->transmissionProperties.upshiftRPM = engineTransmission_json.value( "upshiftRPM", engineTransmission->transmissionProperties.upshiftRPM );
		engineTransmission->transmissionProperties.primaryTransmissionRatio = engineTransmission_json.value( "primaryTransmissionRatio", engineTransmission->transmissionProperties.primaryTransmissionRatio );
		{
			std::vector<float> meow = engineTransmission_json.value("wheelTorqueRatio", std::vector<float>());
			for (int i = 0; i < meow.size() && i < engineTransmission->transmissionProperties.wheelsTorqueRatio.size; ++i) {
				engineTransmission->transmissionProperties.wheelsTorqueRatio.Data[i] = meow[i];
			}
		}
	}
}

void DoCarHandlingUI(jc3::CVehicle *real_vehicle, jc3::CPfxVehicle *pfxVehicle) {

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

	ImGui::DragFloat("Drag Coefficient", &real_vehicle->DragCoefficient);
	ImGui::DragFloat("Mass", &real_vehicle->Mass);
	ImGui::DragFloat("Linear Damping", &real_vehicle->LinearDamping);
	ImGui::DragFloat("Angular Damping", &real_vehicle->AngularDamping);
	ImGui::SliderFloat("Gravity Factor", &real_vehicle->GravityFactor, -128, 128);

	ImGui::Separator();

	if (ImGui::CollapsingHeader("Gravity Modifiers")) {
		ImGui::TreePush("Gravity Modifiers");
		ImGui::DragFloat("Gravity Grounded", &pfxCar->someGravityModifiers->gravityMultiplierGrounded);
		ImGui::TreePop();
	}

	if (ImGui::CollapsingHeader("Engine"))
	{
		ImGui::TreePush("Engine");
		auto engine = pfxCar->landVehicleEngine;
		ImGui::Checkbox("Is Clutching", (bool*)&engine->isClutching);
		ImGui::DragFloat("Clutch delay", &engine->clutchDelay);
		ImGui::DragFloat("Clutching Time", &engine->clutchingTime);
		ImGui::DragFloat("Clutch amount", &engine->clutchAmount);
		ImGui::DragFloat("Manual Clutch Engage Timer", &engine->manualClutchEngageTimer);
		ImGui::DragFloat("Source clutch Rpm", &engine->sourceClutchRpm);
		ImGui::DragFloat("Target Clutch Rpm", &engine->targetClutchRpm);
		ImGui::DragFloat("Engine Revs", &engine->engineRevs);
		ImGui::DragFloat("Engine Damage", &engine->engineDamage);
		ImGui::DragFloat("Rev Limiter Magnitude RPM", &engine->revLimiterMagnitudeRPM);
		ImGui::Checkbox("Is Rev Limiting", (bool*)&engine->isRevLimiting);
		ImGui::DragFloat("Full Load Torque", &engine->fullLoadTorque);
		ImGui::DragFloat("Lowest Max Torque", &engine->lowestMaxTorque);
		ImGui::DragFloat("Engine Min Noise", &engine->engineMinNoise);
		ImGui::DragFloat("Engine Damage Noise Scale", &engine->engineDamageNoiseScale);
		ImGui::DragFloat("Engine Max Damage Torque Factor", &engine->engineMaxDamageTorqueFactor);
		ImGui::DragFloat("Min RPM", &engine->minRPM);
		ImGui::DragFloat("Optimal RPM", &engine->optRPM);
		ImGui::DragFloat("Max Torque", &engine->maxTorque);
		ImGui::DragFloat("Torque Factor at Min RPM", &engine->torqueFactorAtMinRPM);
		ImGui::DragFloat("Torque Factor at Max RPM", &engine->torqueFactorAtMaxRPM);
		ImGui::DragFloat("Resistance Factor at Min RPM", &engine->resistanceFactorAtMinRPM);
		ImGui::DragFloat("Resistance Factor at Optimal RPM", &engine->resistanceFactorAtOptRPM);
		ImGui::DragFloat("Resistance Factor at Max RPM", &engine->resistanceFactorAtMaxRPM);
		ImGui::DragFloat("Clutch Slop RPM", &engine->clutchSlipRPM);
		ImGui::DragFloat("Max RPM", &engine->maxRPM);
		ImGui::DragFloat("Overdrive Max RPM", &engine->overdriveMaxRPM);
		ImGui::Checkbox("Overdrive Active", (bool*)&engine->isOverdriveActive);
		ImGui::TreePop();
	}

	if (ImGui::CollapsingHeader("Engine Transmission")) {
		ImGui::TreePush("Engine Transmission");
		auto engineTransmission = pfxCar->landVehicleTransmission;

		ImGui::SliderFloat("Forward Torque Ratio", &engineTransmission->transmissionProperties.forwardTorqueRatio, 0, 128);
		ImGui::SliderFloat("Low Gear Forward Torque Ratio", &engineTransmission->transmissionProperties.lowGearForwardTorqueRatio, 0, 128);
		ImGui::SliderFloat("Max Transmission RPM", &engineTransmission->transmissionProperties.maxTransmissionRPM, 0, 128);
		ImGui::SliderFloat("Max Reverse Transmission RPM", &engineTransmission->transmissionProperties.maxReversingTransmissionRPM, 0, 128);
		ImGui::SliderFloat("Target Cruise RPM", &engineTransmission->transmissionProperties.targetCruiseRPM, 0, 50000);
		ImGui::SliderFloat("Decay Time to Cruise RPM", &engineTransmission->transmissionProperties.decayTimeToCruiseRPM, -10, 128);
		ImGui::SliderFloat("Low Gearing Primary Transmission Ratio", &engineTransmission->transmissionProperties.lowGearingPrimaryTransmissionRatio, -10, 128);
		ImGui::SliderFloat("Downshift RPM", &engineTransmission->transmissionProperties.downshiftRPM, 0, 50000);
		ImGui::SliderFloat("Upshift RPM", &engineTransmission->transmissionProperties.upshiftRPM, 0, 50000);
		ImGui::SliderFloat("Primary Transmission Ratio", &engineTransmission->transmissionProperties.primaryTransmissionRatio, 0, 128);
		for (int i = 0; i < engineTransmission->transmissionProperties.wheelsTorqueRatio.size; ++i) {
			char wheel_text[100];
			sprintf(wheel_text, "Wheel Torque Ratio %d", i);
			ImGui::TreePush(wheel_text);
			ImGui::SliderFloat(wheel_text, &engineTransmission->transmissionProperties.wheelsTorqueRatio.Data[i], 0, 128);
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}

	if (ImGui::CollapsingHeader("Suspension")) {
		ImGui::TreePush("Suspension");
		for (int i = 0; i < pfxCar->wheelInfo.size; ++i) {
			auto & suspension = pfxCar->wheelSuspensions[i];
			char wheel_text[100];
			sprintf(wheel_text, "Suspension Wheel %d", i);
			if (ImGui::CollapsingHeader(wheel_text)) {
				ImGui::TreePush(wheel_text);
				ImGui::DragFloat("Suspension Force Mag At Rest", &suspension.suspensionForceMagnitudeAtRest);
				ImGui::DragFloat("Suspension Length At Rest", &suspension.suspensionLengthAtRest);
				if (ImGui::CollapsingHeader("Properties")) {
					auto properties = suspension.suspensionProperties;
					ImGui::DragFloat("Antirollbar Strength", &properties->antiRollbarStrength);
					ImGui::DragFloat("Compression", &properties->compression);
					ImGui::DragFloat("Length", &properties->length);
					ImGui::DragFloat("Relaxation", &properties->relaxation);
					ImGui::DragFloat("Strength", &properties->strength);
					ImGui::DragFloat("Lateral Tire Force Offset", &properties->lateralTireForceOffset);
					ImGui::DragFloat("Longtid Tire Force Offset", &properties->longitudinalTireForceOffse);
					ImGui::DragFloat("Tire drag Force offset", &properties->tireDragForceOffset);
					ImGui::DragFloat("Hardpoint offset along spring", &properties->hardpointOffsetOnSpring);
				}
				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}

	if (ImGui::CollapsingHeader("Brakes")) {
		ImGui::TreePush("Brakes Front");
		ImGui::Text("Front");
		ImGui::Checkbox("Handbrake", (bool*)&pfxCar->brakesProperties->front.handbrake);
		// Just making sure all the memory is set :D
		if (*(bool*)&pfxCar->brakesProperties->front.handbrake) {
			pfxCar->brakesProperties->front.handbrake = 1;
		}
		else {
			pfxCar->brakesProperties->front.handbrake = 0;
		}
		ImGui::DragFloat("Max Brake Torque", &pfxCar->brakesProperties->front.maxTorque);
		ImGui::DragFloat("Time To Block", &pfxCar->brakesProperties->front.minTimeToBlock);
		ImGui::TreePop();
		ImGui::Separator();
		ImGui::Text("Rear");
		ImGui::TreePush("Brakes Rear");
		ImGui::Checkbox("Handbrake", (bool*)&pfxCar->brakesProperties->rear.handbrake);
		// Just making sure all the memory is set :D
		if (*(bool*)&pfxCar->brakesProperties->rear.handbrake) {
			pfxCar->brakesProperties->rear.handbrake = 1;
		}
		else {
			pfxCar->brakesProperties->rear.handbrake = 0;
		}
		ImGui::DragFloat("Max Brake Torque", &pfxCar->brakesProperties->rear.maxTorque);
		ImGui::DragFloat("Time To Block", &pfxCar->brakesProperties->rear.minTimeToBlock);
		ImGui::TreePop();
	}

	if (ImGui::CollapsingHeader("Aerodynamics")) {
		ImGui::TreePush("Aerodynamic");
		ImGui::DragFloat("Air Density", &pfxCar->carAerodynamics->airDensity);
		ImGui::DragFloat("Frontal Area", &pfxCar->carAerodynamics->frontalArea);
		ImGui::DragFloat("Drag Coefficient", &pfxCar->carAerodynamics->dragCoefficient);
		ImGui::DragFloat("Top Speed Drag Coefficient", &pfxCar->carAerodynamics->topSpeedDragCoefficient);
		ImGui::DragFloat("Lift Coefficient", &pfxCar->carAerodynamics->liftCoefficient);
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

	//util::hooking::func_call<void>(0x143794F60, real_vehicle);
	util::hooking::func_call<void>(0x1434A64B0, pfxCar);
}
