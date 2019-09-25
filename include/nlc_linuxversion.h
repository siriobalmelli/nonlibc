#ifndef nlc_linuxversion_h_
#define nlc_linuxversion_h_

/*	nlc_linuxversion.h
 * This is a purely internal header, not exported.
 * It exists to allow Nix to replace this #include statement with
 * any _actual_ defines it finds outside the sandbox.
 *
 * See default.nix for this patching code.
 *
 * Otherwise, we get absurd situations like a 3.10 kernel
 * reporting version 4.17.
 *
 * If you look at this and think "No man! There is a much simpler way!",
 * please shoot me an e-mail, I'm out of ideas.
 *
 * Sirio Balmelli
 */
#include <linux/version.h>

#endif /* nlc_linuxversion_h_ */
