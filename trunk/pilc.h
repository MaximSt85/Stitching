#ifndef PILC_H_
#define PILC_H_

class pilc {
public:
	pilc(string path_to_device);
	void set_arm(int on_off);
	void set_nbTriggers(int nbTriggers_to_set);
	void set_positionTriggerStart(double positionTriggerStart_to_set);
	void set_positionTriggerStepSize(double positionTriggerStepSize_to_set);
	vector<double> get_position1Data();
	void set_triggerMode(int triggerMode_to_set);
	void calibrateEncoder(int encoderNumber, double position_to_calibrate);
private:
	Tango::DeviceProxy *pilc_device;	
	Tango::DeviceAttribute nbTriggers_attribute;
	Tango::DeviceAttribute positionTriggerStart_attribute;
	Tango::DeviceAttribute arm_attribute;
	Tango::DeviceAttribute timeTriggerStepSize_attribute;
	Tango::DeviceAttribute positionTriggerStepSize_attribute;
	Tango::DeviceAttribute position1Data_attribute;
	Tango::DeviceAttribute triggerMode_attribute;

	Tango::DeviceData d_in,d_out;

	double positionTriggerStart;
	long arm;
	int nbTriggers;
	double timeTriggerStepSize;
	double positionTriggerStepSize;
	double calibrateEncoder1;
	vector<double> position1Data;
	int triggerMode;

	string nbTriggers_name = "NbTriggers";
	string positionTriggerStart_name = "PositionTriggerStart";
	string arm_name = "Arm";
	string timeTriggerStepSize_name = "TimeTriggerStepSize";
	string positionTriggerStepSize_name = "PositionTriggerStepSize";
	string calibrateEncoder1_name = "CalibrateEncoder1";
	string position1Data_name = "Position1Data";
	string triggerMode_name = "TriggerMode";
};
#endif
