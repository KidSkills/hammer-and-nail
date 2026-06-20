#include <stdio.h>
#include <string.h>

#include "../connector/api.h"
#include "../include/arena.h"
#include "../include/logger.h"
#include "../include/sandbox.h"

typedef int (*self_test_fn)(void);

typedef struct {
    const char *name;
    self_test_fn run;
} self_test_case_t;

static int passed = 0;
static int failed = 0;
static int skipped = 0;

static int self_test_allocator(void)
{
    arena_t *arena = arena_create(4096, ARENA_ZERO_INIT);
    if (arena == NULL) {
        fprintf(stderr, "arena_create returned NULL\n");
        return 1;
    }

    char *block = arena_alloc(arena, 64);
    if (block == NULL) {
        fprintf(stderr, "arena_alloc returned NULL\n");
        arena_destroy(arena);
        return 1;
    }

    for (size_t i = 0; i < 64; i++) {
        if (block[i] != '\0') {
            fprintf(stderr, "arena_alloc did not zero-initialize byte %zu\n", i);
            arena_destroy(arena);
            return 1;
        }
    }

    strcpy(block, "allocator self-test");
    if (!arena_contains(arena, block)) {
        fprintf(stderr, "arena_contains did not recognize allocated block\n");
        arena_destroy(arena);
        return 1;
    }

    arena_stats_t stats = arena_get_stats(arena);
    if (stats.allocation_count != 1 || stats.current_usage == 0) {
        fprintf(stderr, "arena stats were not updated after allocation\n");
        arena_destroy(arena);
        return 1;
    }

    if (arena_total_capacity(arena) < 4096) {
        fprintf(stderr, "arena capacity is smaller than requested region size\n");
        arena_destroy(arena);
        return 1;
    }

    arena_reset(arena);
    stats = arena_get_stats(arena);
    if (stats.current_usage != 0) {
        fprintf(stderr, "arena_reset did not clear current usage\n");
        arena_destroy(arena);
        return 1;
    }

    arena_destroy(arena);
    return 0;
}

static int self_test_connector(void)
{
    connector_config_t config;
    memset(&config, 0, sizeof(config));
    config.config_version = CONNECTOR_CONFIG_VERSION;
    config.struct_size = sizeof(config);
    config.mode = CONNECTOR_MODE_SYNC;
    config.timeout_ms = 1000;
    config.max_concurrency = 1;
    config.receive_buffer_size = 4096;
    config.send_buffer_size = 4096;
    config.max_message_size = 4096;
    config.encoding = CONNECTOR_ENCODING_BINARY;
    config.compression = CONNECTOR_COMPRESSION_NONE;

    connector_result_t result = connector_init(&config);
    if (result != CONNECTOR_SUCCESS) {
        fprintf(stderr, "connector_init failed: %d\n", (int)result);
        return 1;
    }

    connector_buffer_t *buffer = connector_buffer_alloc(32);
    if (buffer == NULL) {
        fprintf(stderr, "connector_buffer_alloc returned NULL\n");
        connector_shutdown();
        return 1;
    }

    const char payload[] = "frailbox self-test";
    memcpy(buffer->data, payload, sizeof(payload));
    buffer->size = sizeof(payload);

    result = connector_send(buffer);
    if (result != CONNECTOR_SUCCESS) {
        fprintf(stderr, "connector_send failed: %d\n", (int)result);
        connector_buffer_free(buffer);
        connector_shutdown();
        return 1;
    }

    result = connector_buffer_free(buffer);
    if (result != CONNECTOR_SUCCESS) {
        fprintf(stderr, "connector_buffer_free failed: %d\n", (int)result);
        connector_shutdown();
        return 1;
    }

    result = connector_shutdown();
    if (result != CONNECTOR_SUCCESS) {
        fprintf(stderr, "connector_shutdown failed: %d\n", (int)result);
        return 1;
    }

    return 0;
}

static int self_test_logger(void)
{
    if (log_init() != 0) {
        fprintf(stderr, "log_init failed\n");
        return 1;
    }

    log_set_level(LOG_LEVEL_DEBUG);
    if (log_get_level() != LOG_LEVEL_DEBUG) {
        fprintf(stderr, "log_get_level did not report debug level\n");
        log_shutdown();
        return 1;
    }

    LOG_INFO("frailbox self-test logger smoke check");
    LOG_DEBUG("frailbox self-test debug check");
    log_shutdown();
    return 0;
}

static int self_test_sandbox(void)
{
    sandbox_config_t config;
    memset(&config, 0, sizeof(config));
    config.type = SANDBOX_NONE;
    config.memory_limit_bytes = 0;
    config.cpu_limit_ns = 0;
    config.max_processes = 0;
    config.max_open_fds = 0;
    config.enable_network = 0;
    config.enable_ptrace = 0;

    sandbox_t *sandbox = sandbox_create(&config);
    if (sandbox == NULL) {
        fprintf(stderr, "sandbox_create returned NULL\n");
        return 1;
    }

    if (sandbox_apply(sandbox) != 0) {
        fprintf(stderr, "sandbox_apply failed\n");
        sandbox_destroy(sandbox);
        return 1;
    }

    if (!sandbox_is_active(sandbox)) {
        fprintf(stderr, "sandbox did not become active\n");
        sandbox_destroy(sandbox);
        return 1;
    }

    sandbox_destroy(sandbox);
    return 0;
}

static void run_case(const self_test_case_t *test)
{
    printf("  %-28s", test->name);
    fflush(stdout);

    int result = test->run();
    if (result == 0) {
        passed++;
        printf("PASS\n");
    } else if (result == 77) {
        skipped++;
        printf("SKIP\n");
    } else {
        failed++;
        printf("FAIL\n");
    }
}

int main(void)
{
    const self_test_case_t tests[] = {
        {"allocator basics", self_test_allocator},
        {"connector basic API", self_test_connector},
        {"logger initialization", self_test_logger},
        {"sandbox initialization", self_test_sandbox},
    };
    const size_t test_count = sizeof(tests) / sizeof(tests[0]);

    printf("frailbox self-test\n");
    printf("==================\n");

    for (size_t i = 0; i < test_count; i++) {
        run_case(&tests[i]);
    }

    printf("\nsummary: passed=%d failed=%d skipped=%d total=%zu\n",
           passed, failed, skipped, test_count);

    return failed == 0 ? 0 : 1;
}
