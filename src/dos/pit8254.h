#ifndef PIT8254_H_
#define PIT8254_H_

/* frequency of the oscillator driving the 8254 timer */
#define OSC_FREQ_HZ		1193182

/* I/O ports connected to the 8254 */
#define PORT_DATA0	0x40
#define PORT_DATA1	0x41
#define PORT_DATA2	0x42
#define PORT_CMD	0x43

/* command bits */
#define CMD_CHAN0			0
#define CMD_CHAN1			(1 << 6)
#define CMD_CHAN2			(2 << 6)
#define CMD_RDBACK			(3 << 6)

#define CMD_LATCH			0
#define CMD_ACCESS_LOW		(1 << 4)
#define CMD_ACCESS_HIGH		(2 << 4)
#define CMD_ACCESS_BOTH		(3 << 4)

#define CMD_OP_INT_TERM		0
#define CMD_OP_ONESHOT		(1 << 1)
#define CMD_OP_RATE			(2 << 1)
#define CMD_OP_SQWAVE		(3 << 1)
#define CMD_OP_SW_STROBE	(4 << 1)
#define CMD_OP_HW_STROBE	(5 << 1)

#define CMD_MODE_BIN		0
#define CMD_MODE_BCD		1

#endif	/* PIT8254_H_ */
