// solo per compilare

static inline void __BKPT(int x) {}
static inline uint32_t __REV16(uint16_t x) { return 0 ; }

static inline void CyDelay(int x) {}

#define CY_INT_HARD_FAULT_IRQN	0
#define CY_INT_SYSTICK_IRQN	0

#define CyGlobalIntDisable	0
#define CyGlobalIntEnable	0

typedef void (*PF_IRQ)(void) ;
static inline PF_IRQ CyIntSetSysVector(int x, PF_IRQ y) { return NULL ; }

static inline void CySysTickClear(void) {}
static inline void CySysTickEnable(void) {}
static inline void CySysTickStart(void) {}
static inline void CySysTickStop(void) {}

typedef uint16_t CYBLE_GATT_DB_ATTR_HANDLE_T ;

static inline void CySysClkEcoStop(void) {}

#define CY_NOINIT


