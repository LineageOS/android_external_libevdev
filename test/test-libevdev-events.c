/*
 * Copyright © 2013 Red Hat, Inc.
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that copyright
 * notice and this permission notice appear in supporting documentation, and
 * that the name of the copyright holders not be used in advertising or
 * publicity pertaining to distribution of the software without specific,
 * written prior permission.  The copyright holders make no representations
 * about the suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 * THE COPYRIGHT HOLDERS DISCLAIM ALL WARRANTIES WITH REGARD TO THIS SOFTWARE,
 * INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS, IN NO
 * EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE FOR ANY SPECIAL, INDIRECT OR
 * CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE,
 * DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 * TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE
 * OF THIS SOFTWARE.
 */

#define _GNU_SOURCE
#include <config.h>
#include <libevdev/libevdev.h>
#include <linux/input.h>

#include <check.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include "test-common-uinput.h"

START_TEST(test_next_event)
{
	struct uinput_device* uidev;
	struct libevdev *dev;
	int rc;
	int fd;
	struct input_event ev;

	rc = uinput_device_new_with_events(&uidev, "test device", DEFAULT_IDS,
					   EV_REL, REL_X,
					   EV_REL, REL_Y,
					   EV_KEY, BTN_LEFT,
					   -1);
	ck_assert_msg(rc == 0, "Failed to create uinput device: %s", strerror(-rc));

	fd = uinput_device_get_fd(uidev);
	rc = fcntl(fd, F_SETFL, O_NONBLOCK);
	ck_assert_int_eq(rc, 0);
	rc = libevdev_new_from_fd(fd, &dev);
	ck_assert_msg(rc == 0, "Failed to init device: %s", strerror(-rc));;

	rc = libevdev_next_event(dev, LIBEVDEV_READ_NORMAL, &ev);
	ck_assert_int_eq(rc, -EAGAIN);

	uinput_device_event(uidev, EV_KEY, BTN_LEFT, 1);
	uinput_device_event(uidev, EV_SYN, SYN_REPORT, 0);
	rc = libevdev_next_event(dev, LIBEVDEV_READ_NORMAL, &ev);
	ck_assert_int_eq(rc, 0);
	ck_assert_int_eq(ev.type, EV_KEY);
	ck_assert_int_eq(ev.code, BTN_LEFT);
	ck_assert_int_eq(ev.value, 1);

	libevdev_free(dev);
	uinput_device_free(uidev);

}
END_TEST

START_TEST(test_syn_event)
{
	struct uinput_device* uidev;
	struct libevdev *dev;
	int rc;
	int fd;
	struct input_event ev;
	int pipefd[2];

	rc = uinput_device_new_with_events(&uidev, "test device", DEFAULT_IDS,
					   EV_SYN, SYN_REPORT,
					   EV_SYN, SYN_DROPPED,
					   EV_REL, REL_X,
					   EV_REL, REL_Y,
					   EV_KEY, BTN_LEFT,
					   -1);
	ck_assert_msg(rc == 0, "Failed to create uinput device: %s", strerror(-rc));

	fd = uinput_device_get_fd(uidev);
	rc = fcntl(fd, F_SETFL, O_NONBLOCK);
	ck_assert_int_eq(rc, 0);
	rc = libevdev_new_from_fd(fd, &dev);
	ck_assert_msg(rc == 0, "Failed to init device: %s", strerror(-rc));;

	/* This is a bid complicated:
	   we can't get SYN_DROPPED through uinput, so we push two events down
	   uinput, and fetch one off libevdev (reading in the other one on the
	   way). Then write a SYN_DROPPED on a pipe, switch the fd and read
	   one event off the wire (but returning the second event from
	   before). Switch back, so that when we do read off the SYN_DROPPED
	   we have the fd back on the device and the ioctls work.
	 */
	uinput_device_event(uidev, EV_KEY, BTN_LEFT, 1);
	uinput_device_event(uidev, EV_SYN, SYN_REPORT, 0);
	rc = libevdev_next_event(dev, LIBEVDEV_READ_NORMAL, &ev);
	ck_assert_int_eq(rc, 0);
	ck_assert_int_eq(ev.type, EV_KEY);
	ck_assert_int_eq(ev.code, BTN_LEFT);

	rc = pipe2(pipefd, O_NONBLOCK);
	ck_assert_int_eq(rc, 0);

	libevdev_change_fd(dev, pipefd[0]);
	ev.type = EV_SYN;
	ev.code = SYN_DROPPED;
	ev.value = 0;
	rc = write(pipefd[1], &ev, sizeof(ev));
	ck_assert_int_eq(rc, sizeof(ev));
	rc = libevdev_next_event(dev, LIBEVDEV_READ_NORMAL, &ev);

	libevdev_change_fd(dev, fd);

	ck_assert_int_eq(rc, 0);
	ck_assert_int_eq(ev.type, EV_SYN);
	ck_assert_int_eq(ev.code, SYN_REPORT);
	rc = libevdev_next_event(dev, LIBEVDEV_READ_NORMAL, &ev);
	ck_assert_int_eq(rc, 1);

	/* only check for the rc, nothing actually changed on the device */

	libevdev_free(dev);
	uinput_device_free(uidev);

	close(pipefd[1]);
	close(pipefd[2]);

}
END_TEST

Suite *
libevdev_events(void)
{
	Suite *s = suite_create("libevdev event tests");

	TCase *tc = tcase_create("event polling");
	tcase_add_test(tc, test_next_event);
	tcase_add_test(tc, test_syn_event);
	suite_add_tcase(s, tc);


	return s;
}
