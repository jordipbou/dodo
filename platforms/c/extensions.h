#include <sloth.h>

/* Memory-stack transfer operations */

typedef int8_t BYTE;
typedef int16_t WYDE;
typedef int32_t LONG;
typedef int64_t EXTENDED;

void sloth_b_fetch_(X* x);
void sloth_b_store_(X* x);
void sloth_w_fetch_(X* x);
void sloth_w_store_(X* x);
void sloth_l_fetch_(X* x);
void sloth_l_store_(X* x);
void sloth_x_fetch_(X* x);
void sloth_x_store_(X* x);

void sloth_ints_(X* x);

void sloth_bootstrap_extensions(X* x);
