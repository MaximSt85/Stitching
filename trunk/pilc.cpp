#include <tango.h>
#include "pilc.h"

pilc::pilc(string path_to_device) {
	try {
		pilc_device = new Tango::DeviceProxy(path_to_device);
		arm_attribute = pilc_device->read_attribute(arm_name);
		nbTriggers_attribute = pilc_device->read_attribute(nbTriggers_name);
		positionTriggerStart_attribute = pilc_device->read_attribute(positionTriggerStart_name);
		positionTriggerStepSize_attribute = pilc_device->read_attribute(positionTriggerStepSize_name);
		triggerMode_attribute = pilc_device->read_attribute(triggerMode_name);
	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
    }
}

void pilc::set_arm(int on_off) {
	if (on_off == 1) {
		try {
			//arm_attribute = pilc_device->read_attribute(arm_name);
			//arm_attribute >> arm;
			Tango::DevVarLongArray *in = new Tango::DevVarLongArray();
			in->length(2);
			(*in)[0] = 1;
			(*in)[1] = 1;
			arm_attribute << in;
			pilc_device->write_attribute(arm_attribute);
  		}
		catch (Tango::DevFailed &e) {
			Tango::Except::print_exception(e);
	    }
	}
	else {
		try {
			//arm_attribute = pilc_device->read_attribute(arm_name);
			//arm_attribute >> arm;
			Tango::DevVarLongArray *in = new Tango::DevVarLongArray();
			in->length(2);
			(*in)[0] = 1;
			(*in)[1] = 0;
			arm_attribute << in;
			pilc_device->write_attribute(arm_attribute);
  		}
		catch (Tango::DevFailed &e) {
			Tango::Except::print_exception(e);
	    }
	}
}

void pilc::set_nbTriggers(int nbTriggers_to_set) {
	try {
		//nbTriggers_attribute = pilc_device->read_attribute(nbTriggers_name);
		//nbTriggers_attribute >> nbTriggers;
		nbTriggers = nbTriggers_to_set;
		nbTriggers_attribute << nbTriggers;
		pilc_device->write_attribute(nbTriggers_attribute);
	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
	}
}

void pilc::set_positionTriggerStart(double positionTriggerStart_to_set) {
	try {
		//positionTriggerStart_attribute = pilc_device->read_attribute(positionTriggerStart_name);
		//positionTriggerStart_attribute >> positionTriggerStart;
		positionTriggerStart = positionTriggerStart_to_set;
		positionTriggerStart_attribute << positionTriggerStart;
		pilc_device->write_attribute(positionTriggerStart_attribute);
	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
	}
}

void pilc::set_positionTriggerStepSize(double positionTriggerStepSize_to_set) {
	try {
		//positionTriggerStepSize_attribute = pilc_device->read_attribute(positionTriggerStepSize_name);
		//positionTriggerStepSize_attribute >> positionTriggerStepSize;
		positionTriggerStepSize = positionTriggerStepSize_to_set;
		positionTriggerStepSize_attribute << positionTriggerStepSize;
		pilc_device->write_attribute(positionTriggerStepSize_attribute);
	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
	}
}

vector<double> pilc::get_position1Data() {
	try {
		position1Data_attribute = pilc_device->read_attribute(position1Data_name);
		position1Data_attribute >> position1Data;
		return position1Data;
	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
	}
}

void pilc::set_triggerMode(int triggerMode_to_set) {
	try {
		//nbTriggers_attribute = pilc_device->read_attribute(nbTriggers_name);
		//nbTriggers_attribute >> nbTriggers;
		triggerMode = triggerMode_to_set;
		triggerMode_attribute << triggerMode;
		pilc_device->write_attribute(triggerMode_attribute);
	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
	}
}

void pilc::calibrateEncoder(int encoderNumber, double position_to_calibrate) {
	string command_name = "CalibrateEncoder1";
	try {
		if (encoderNumber == 1) {
			command_name = "CalibrateEncoder1";
		}
		else if (encoderNumber == 2) {
			command_name = "CalibrateEncoder2";
		}
		else if (encoderNumber == 3) {
			command_name = "CalibrateEncoder3";
		}
		else if (encoderNumber == 4) {
			command_name = "CalibrateEncoder4";
		}
		else if (encoderNumber == 5) {
			command_name = "CalibrateEncoder5";
		}
		else {
			throw invalid_argument( "Encoder number must be more or equal 1 and less or equal 5" );
		}
		d_in << position_to_calibrate;
		pilc_device->command_inout(command_name, d_in);
	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
	}
}


