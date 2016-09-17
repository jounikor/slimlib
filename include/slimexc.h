// ============================================================================
//
// v0.1 (c) 2011 Jouni 'Mr.Spiv' Korhonen
//
// This is not GPL.
//
//
//
//
// ============================================================================
#ifndef _sl4exceptions_h
#define _sl4exceptions_h
//=============================================================================


/*
 *  slimexc.h
 *  sl4lib
 *
 *  Created by Jouni Korhonen on 1/30/11.
 *  Copyright 2011 __MyCompanyName__. All rights reserved.
 *
 */

class USBError {
protected:
	const char* m_what;
public:
	int error;
	int line;
	const char* file;
	USBError() throw(): m_what(NULL),error(0),line(-1),file("-- ") {}
	virtual ~USBError() throw() {}
	virtual const char* what() const throw() { return m_what; }
};

//=============================================================================

class USBDeviceNotFound: public USBError {
public:
	USBDeviceNotFound(const char* s=NULL) throw(): USBError() {m_what=s;}
	USBDeviceNotFound(int l, const char* f, const char* s=NULL) throw(): USBError() {
		line = l; file = f;	m_what=s;
	}
	~USBDeviceNotFound() throw() {}
	const char* what() const throw() {
		if (m_what) {
			return m_what;
		} else {
			return "USBDeviceNotFound()";
		}
	}
};

class USBDeviceInitError: public USBError {
public:
	USBDeviceInitError(const char* s=NULL) throw(): USBError() {m_what=s;}
	USBDeviceInitError(int l, const char* f, const char* s=NULL) throw(): USBError() {
		line = l; file = f;	m_what=s;
	}
	~USBDeviceInitError() throw() {}
	const char* what() const throw() {
		if (m_what) {
			return m_what;
		} else {
			return "USBDeviceInitError()";
		}
	}
};

class USBDeviceInUseError: public USBError {
public:
	USBDeviceInUseError(const char* s=NULL) throw() {m_what=s;}
	USBDeviceInUseError(int l, const char* f, const char* s=NULL) throw(): USBError() {
		line = l; file = f;	m_what=s;
	}
	~USBDeviceInUseError() throw() {}
	const char* what() const throw() {
		if (m_what) {
			return m_what;
		} else {
			return "USBDeviceInUseError()";
		}
	}
};


class USBReadBulkError: public USBError {
public:
	USBReadBulkError(const char* s=NULL) throw(): USBError() {m_what=s;}
	USBReadBulkError(int l, char* f, const char* s=NULL) throw(): USBError() {
		line = l; file = f;	m_what=s;
	}
	USBReadBulkError(int l, const char* f, int p, const char* s=NULL) throw(): USBError() {
		line = l; file = f;	error = p; m_what=s;
	}
	~USBReadBulkError() throw() {}
	const char* what() const throw() {
		if (m_what) {
			return m_what;
		} else {
			return "USBReadBulkError()";
		}
	}
};

class USBWriteBulkError: public USBError {
public:
	USBWriteBulkError(const char* s=NULL) throw(): USBError() {m_what=s;}
	USBWriteBulkError(int l, const char* f, const char* s=NULL) throw(): USBError() {
		line = l; file = f;	m_what=s;
	}
	USBWriteBulkError(int l, const char* f, int p, const char* s=NULL) throw(): USBError() {
		line = l; file = f;	error = p; m_what=s;
	}
	~USBWriteBulkError() throw() {}
	const char* what() const throw() {
		if (m_what) {
			return m_what;
		} else {
			return "USBWriteBulkError()";
		}
	}
};

class USBReadCtrlError: public USBError {
public:
	USBReadCtrlError(const char* s=NULL) throw(): USBError() {m_what=s;}
	USBReadCtrlError(int l, const char* f, const char* s=NULL) throw(): USBError() {
		line = l; file = f;	m_what=s;
	}
	USBReadCtrlError(int l, const char* f, int p, const char* s=NULL) throw(): USBError() {
		line = l; file = f; error = p;	m_what=s;
	}
	~USBReadCtrlError() throw() {}
	const char* what() const throw() {
		if (m_what) {
			return m_what;
		} else {
			return "USBReadCtrlError()";
		}
	}
};

class USBWriteCtrlError: public USBError {
public:
	USBWriteCtrlError(const char* s=NULL) throw(): USBError() {m_what=s;}
	USBWriteCtrlError(int l, const char* f, const char* s=NULL) throw(): USBError() {
		line = l; file = f;	m_what=s;
	}
	USBWriteCtrlError(int l, const char* f, int p, const char* s=NULL) throw(): USBError() {
		line = l; file = f;	m_what=s;
	}
	~USBWriteCtrlError() throw() {}
	const char* what() const throw() {
		if (m_what) {
			return m_what;
		} else {
			return "USBWriteCtrlError()";
		}
	}
};

class USBGenericError: public USBError {
public:
	USBGenericError(const char* s=NULL) throw(): USBError() {m_what=s;}
	USBGenericError(int l, const char* f, const char* s=NULL) throw(): USBError() {
		line = l; file = f;	m_what=s;
	}
	~USBGenericError() throw() {}
	const char* what() const throw() {
		if (m_what) {
			return m_what;
		} else {
			return "USBGenericError()";
		}
	}
};

class USBParameterError: public USBError {
public:
	USBParameterError(const char* s=NULL) throw(): USBError() {m_what=s;}
	USBParameterError(int l, const char* f, const char* s=NULL) throw(): USBError() {
		line = l; file = f;	m_what=s;
	}
	USBParameterError(int l, const char* f, int p, const char* s=NULL) throw(): USBError() {
		line = l; file = f;	error = p; m_what=s;
	}
	~USBParameterError() throw() {}
	const char* what() const throw() {
		if (m_what) {
			return m_what;
		} else {
			return "USBParameterError()";
		}
	}
};

class USBWaitError: public USBError {
public:
	USBWaitError(const char* s=NULL) throw(): USBError() {m_what=s;}
	USBWaitError(int l, const char* f, const char* s=NULL) throw(): USBError() {
		line = l; file = f;	m_what=s;
	}
	USBWaitError(int l, const char* f, int p, const char* s=NULL) throw(): USBError() {
		line = l; file = f;	error = p; m_what=s;
	}
	USBWaitError() throw() {}
	const char* what() const throw() {
		if (m_what) {
			return m_what;
		} else {
			return "USBWaitError()";
		}
	}
};



#endif //_sl4exceptions_h