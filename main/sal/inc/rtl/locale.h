/**************************************************************
 * 
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 * 
 *   http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 * 
 *************************************************************/



#ifndef _RTL_LOCALE_H_
#define _RTL_LOCALE_H_

#include <rtl/ustring.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SAL_W32
#	pragma pack(push, 8)
#elif defined(SAL_OS2)
#	pragma pack(push, 4)
#endif

/**
	The implementation structur of a locale. Do not create this structure
	direct. Only use the functions rtl_locale_register and
	rtl_locale_setDefault. The strings Language, Country and Variant
	are constants, so it is not necessary to acquire and release them.
 */
typedef struct _rtl_Locale
{
	/**
	 	Lowercase two-letter ISO 639-1 or three-letter ISO 639-3 code.
	 */
	rtl_uString *	Language;
	/**
	 	uppercase two-letter ISO-3166 code.
	 */
	rtl_uString *	Country;
	/**
	 	Lowercase vendor and browser specific code.
	 */
	rtl_uString *	Variant;
	/**
	 	The merged hash value of the Language, Country and Variant strings.
	 */
	sal_Int32		HashCode;
} rtl_Locale;

#if defined( SAL_W32) ||  defined(SAL_OS2)
#pragma pack(pop)
#endif

/**
	Register a locale from language, country and variant.
	@param language lowercase two-letter ISO 639-1 or three-letter ISO 639-3 code.
	@param country uppercase two-letter ISO-3166 code. May be null.
	@param variant vendor and browser specific code. May be null.
 */
rtl_Locale * SAL_CALL rtl_locale_register( const sal_Unicode * language, const sal_Unicode * country, const sal_Unicode * variant );

/**
	Common method of getting the current default Locale.
	Used for the presentation: menus, dialogs, etc.
	Generally set once when your applet or application is initialized,
	then never reset. (If you do reset the default locale, you
	probably want to reload your GUI, so that the change is reflected
	in your interface.)
	<p>More advanced programs will allow users to use different locales
	for different fields, e.g. in a spreadsheet.
	<BR>Note that the initial setting will match the host system.
 */
rtl_Locale * SAL_CALL rtl_locale_getDefault();

/**
	Sets the default.
	Normally set once at the beginning of applet or application,
	then never reset. <code>setDefault</code> does not reset the host locale.
	@param language lowercase two-letter ISO 639-1 or three-letter ISO 639-3 code.
	@param country uppercase two-letter ISO-3166 code.
	@param variant vendor and browser specific code. See class description.
 */
void SAL_CALL rtl_locale_setDefault( const sal_Unicode * language, const sal_Unicode * country, const sal_Unicode * variant );

/**
	Getter for programmatic name of field,
	a lowercased two-letter ISO 639-1 or three-letter ISO 639-3 code.
	@see #getDisplayLanguage
 */
rtl_uString * SAL_CALL rtl_locale_getLanguage( rtl_Locale * This );

/**
	Getter for programmatic name of field,
	an uppercased two-letter ISO-3166 code.
	@see #getDisplayCountry
 */
rtl_uString * SAL_CALL rtl_locale_getCountry( rtl_Locale * This );

/**
	Getter for programmatic name of field.
	@see #getDisplayVariant
 */
rtl_uString * SAL_CALL rtl_locale_getVariant( rtl_Locale * This );

/**
	Returns the hash code of the locale This.
 */
sal_Int32 SAL_CALL rtl_locale_hashCode( rtl_Locale * This );

/**
	Returns true if the locals are equal, otherwis false.
 */
sal_Int32 SAL_CALL rtl_locale_equals( rtl_Locale * This, rtl_Locale * obj  );

#ifdef __cplusplus
}
#endif

#endif /* _RTL_LOCALE_H_ */


