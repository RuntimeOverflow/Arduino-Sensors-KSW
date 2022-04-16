#include "ValueProvider.h"

template <typename T, typename U>
T *ValueProvider<T, U>::value(void) {
	return cachedValue;
}

template <typename T, typename U>
template <typename S>
ValueProvider<S> *ValueProvider<T, U>::as(void) {
	return reinterpret_cast<ValueProvider<S> *>(this);
}

template <typename T, typename U>
Error ValueProvider<T, U>::read(void) {
	if(!sensor) return E_NO_SENSOR;
	else if(!fetchValue) return E_NO_FETCH;
	else if(isReadyForFetching && !(sensor->*isReadyForFetching)()) return E_NOT_READY;

	if(cachedValue) delete cachedValue;
	cachedValue = new T((sensor->*fetchValue)());

	// TODO: Time calculation

	return E_OK;
}