#pragma once

enum __attribute__((__packed__)) Error {
	E_OK = 0,
	E_GENERIC = 1 << 0,
	E_NOT_FOUND = 1 << 1,
	E_ABSTRACT_METHOD_CALL = 1 << 2,
	E_NOT_READY = 1 << 3,
	E_NO_SENSOR = 1 << 4,
	E_NO_FETCH = 1 << 5,
	E_NO_ACTUATOR = 1 << 6,
	E_CONNECTION_FAILED = 1 << 7,
	E_FAILED_TO_SEND = 1 << 8,
};