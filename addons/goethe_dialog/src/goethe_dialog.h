#ifndef GOETHE_DIALOG_H
#define GOETHE_DIALOG_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>
#include <goethe/goethe_dialog.h>

namespace godot {

class GoetheDialogExtension : public RefCounted {
	GDCLASS(GoetheDialogExtension, RefCounted)

private:
	GoetheDialog* dialog;

protected:
	static void _bind_methods();

public:
	GoetheDialogExtension();
	~GoetheDialogExtension();

	bool load_dialog_from_file(const String& file_path);
	bool load_dialog_from_yaml(const String& yaml_content);
	bool save_dialog_to_file(const String& file_path);
	String save_dialog_to_yaml();
	
	String get_dialog_id();
	int get_node_count();
	Dictionary get_node(int index);
	bool add_node(const Dictionary& node_data);
	bool remove_node(int index);
};

}

#endif // GOETHE_DIALOG_H
```

```

