#include <sloth.h>
#include <cpnbi.h>

int main() {
	cpnbi_init();

	X* x = sloth_new();

	sloth_bootstrap(x);

	sloth_include(x, ROOT_PATH "4th/ans.4th");

	/* Set ROOT PATH */
	sloth_set_root_path(x, ROOT_PATH "4th/");

	sloth_repl(x);
}
