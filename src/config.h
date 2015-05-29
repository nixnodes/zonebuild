/*
 * config.h
 *
 *  Created on: May 28, 2015
 *      Author: reboot
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <endian.h>

#if __BYTE_ORDER == __LITTLE_ENDIAN

#elif __BYTE_ORDER == __BIG_ENDIAN
#else
#error
#endif




#endif /* CONFIG_H_ */
