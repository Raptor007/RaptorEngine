/*
 *  Identifier.cpp
 */

#include "Identifier.h"


// Identifier class code is in the header because it's a template class.


const char *IdentifierError::what() const throw()
{
	return "Identifier: Failed to get next unique ID.";
}
