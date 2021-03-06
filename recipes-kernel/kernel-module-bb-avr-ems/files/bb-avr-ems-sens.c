// SPDX-License-Identifier: GPL-2.0+

/*
 * Test module for the BuilderBot AVR multifunction core driver.
 *
 * Copyright (C) 2018 Michael Allwright
 */


#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/mfd/bb-avr.h>
#include <linux/module.h>
#include <linux/of_device.h>
#include <linux/platform_device.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/iio/buffer.h>
#include <linux/iio/iio.h>
#include <linux/iio/sysfs.h>
#include <linux/iio/trigger_consumer.h>
#include <linux/iio/triggered_buffer.h>

static const struct of_device_id bb_avr_ems_of_match[] = {
	{ .compatible = "ulb,bb-avr-ems-sens" },
	{ /* sentinel */ }
};

/**
 * struct bb_avr_ems - AVR BuilderBot Electromagnet System
 *
 * @avr: Pointer to parent BuilderBot AVR device
 */
struct bb_avr_ems {
	struct bb_avr *avr;
};

static irqreturn_t bb_avr_ems_trigger_handler(int irq, void *p)
{
	struct iio_poll_func *pf = p;
	struct iio_dev *indio_dev = pf->indio_dev;
	struct bb_avr_ems *ems = iio_priv(indio_dev);
	int ret = 0;
	u8 voltage;

	ret = bb_avr_exec(ems->avr, BB_AVR_CMD_GET_EM_ACCUM_VOLTAGE,
			  NULL, 0, &voltage, 1);

	if (ret < 0)
		goto out;
	iio_push_to_buffers_with_timestamp(indio_dev, &voltage,
					   iio_get_time_ns(indio_dev));
out:
	iio_trigger_notify_done(indio_dev->trig);
	return IRQ_HANDLED;
}

static const struct iio_chan_spec bb_avr_ems_channels[] = {
	{
		.type = IIO_VOLTAGE,
		.indexed = true,
		.channel = 0,
		.scan_index = 0,
		.scan_type = {
			.sign = 'u',
			.realbits = 8,
			.storagebits = 8,
			.endianness = IIO_BE,
		},
	},
	IIO_CHAN_SOFT_TIMESTAMP(1),
};

static const unsigned long bb_avr_ems_scan_masks[] = {0x1, 0};

static const struct iio_info bb_avr_ems_info = {};

static int bb_avr_ems_probe(struct platform_device *pdev)
{
	struct iio_dev *indio_dev;
	struct bb_avr_ems *ems;
	int ret = 0;

	indio_dev = devm_iio_device_alloc(&pdev->dev, sizeof(*ems));
	if (!indio_dev)
		return -ENOMEM;
	ems = iio_priv(indio_dev);
	/* set the parent AVR device */
	ems->avr = dev_get_drvdata(pdev->dev.parent);

	/* set up the indio_dev struct */
	dev_set_drvdata(&pdev->dev, indio_dev);
	indio_dev->name = "ems-sens";
	indio_dev->dev.parent = &pdev->dev;
	indio_dev->info = &bb_avr_ems_info;
	indio_dev->direction = IIO_DEVICE_DIRECTION_IN;
	indio_dev->modes = INDIO_BUFFER_SOFTWARE;
	indio_dev->channels = bb_avr_ems_channels;
	indio_dev->num_channels = ARRAY_SIZE(bb_avr_ems_channels);
	indio_dev->available_scan_masks = bb_avr_ems_scan_masks;
	
	ret = iio_triggered_buffer_setup(indio_dev,
					 iio_pollfunc_store_time,
					 bb_avr_ems_trigger_handler,
					 NULL);
	if(ret < 0)
		goto err_out;
	ret = iio_device_register(indio_dev);
err_out:
	return ret;
}

static int bb_avr_ems_remove(struct platform_device *pdev)
{
	struct iio_dev *indio_dev = platform_get_drvdata(pdev);

	iio_triggered_buffer_cleanup(indio_dev);
	iio_device_unregister(indio_dev);

	return 0;
}

static struct platform_driver bb_avr_ems_driver = {
	.probe = bb_avr_ems_probe,
	.remove = bb_avr_ems_remove,
	.driver = {
		.name = "bb-avr-ems-sens",
		.of_match_table = bb_avr_ems_of_match,
	},
};

module_platform_driver(bb_avr_ems_driver);

MODULE_DEVICE_TABLE(of, bb_avr_ems_of_match);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Michael Allwright <allsey87@gmail.com>");
MODULE_DESCRIPTION("BuilderBot AVR Electromagnet System Sensor");
