#ifndef RESOURCE_H
#define RESOURCE_H

#include "ref.h"

#ifdef HAVE_FAM
#	include <fam.h>
#endif /* HAVE_FAM */

class Resource: public Ref<Resource> {
	public:
		Resource(const char* path);
		virtual ~Resource();

		virtual void reload() = 0;

	private:
#ifdef HAVE_FAM
		FAMRequest _fam_request;
#endif /* HAVE_FAM */
};

#endif /* RESOURCE_H */
