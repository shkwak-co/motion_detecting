idf_component_register(SRCS "detecting_main.c"
                            "detecting_task.c"
                            "buzzer_task.c"
                            "light_task.c"
                    EMBED_FILES "detecting_task.h"
                                "buzzer_task.h"
                                "light_task.h"
                    INCLUDE_DIRS ".")
