/*
* Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
*
* Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
* the the people's Republic of China and other countries.
* All Allwinner Technology Co.,Ltd. trademarks are used with permission.
*
* DISCLAIMER
* THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
* IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
* IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
* ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
* ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
* COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
* YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
*
*
* THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
* PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
* WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
* THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
* OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
* STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
* OF THE POSSIBILITY OF SUCH DAMAGE.
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>

// Function to drop system caches
void drop_caches() {
	system("sync");
	system("echo 3 > /proc/sys/vm/drop_caches");
}

// Function to perform the write test using O_DIRECT
double write_test(const char *file_path, size_t size)
{
	srand(time(NULL));

	int fd = open(file_path, O_RDWR | O_CREAT | O_SYNC | O_DIRECT, S_IRUSR | S_IWUSR);
	if (fd == -1) {
		perror("Failed to open file for writing");
		return 0;
	}

	// Allocate aligned buffer for O_DIRECT (usually a 512-byte boundary)
	char *buffer;
	if (posix_memalign((void **)&buffer, 512, size) != 0) {
		perror("Failed to allocate aligned buffer");
		close(fd);
		return 0;
	}

	// Fill the buffer with random values
	for (size_t i = 0; i < size; i++) {
		buffer[i] = rand() % 256; // Random byte between 0-255
	}

	struct timeval start, end;
	gettimeofday(&start, NULL);

	ssize_t written = write(fd, buffer, size);
	if (written != size) {
		perror("Failed to write full buffer");
		free(buffer);
		close(fd);
		return 0;
	}

	gettimeofday(&end, NULL);

	close(fd);
	free(buffer);

	// Calculate write speed in MB/s
	double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
	double speed = size / elapsed / (1024 * 1024); // MB/s

	return speed;
}

// Function to perform the read test using O_DIRECT
double read_test(const char *file_path, size_t size)
{
	int fd = open(file_path, O_RDONLY | O_DIRECT);
	if (fd == -1) {
		perror("Failed to open file for reading");
		return 0;
	}

	// Allocate aligned buffer for O_DIRECT (usually a 512-byte boundary)
	char *buffer;
	if (posix_memalign((void **)&buffer, 512, size) != 0) {
		perror("Failed to allocate aligned buffer");
		close(fd);
		return 0;
	}

	struct timeval start, end;
	gettimeofday(&start, NULL);

	ssize_t read_bytes = read(fd, buffer, size);
	if (read_bytes != size) {
		perror("Failed to read full buffer");
		free(buffer);
		close(fd);
		return 0;
	}

	gettimeofday(&end, NULL);

	close(fd);
	free(buffer);

	// Calculate read speed in MB/s
	double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
	double speed = size / elapsed / (1024 * 1024); // MB/s

	return speed;
}

int main()
{
	// Define the file sizes to test (in MB)
	int sizes[] = { 1, 10, 50, 100, 200 };
	int count = 10; // Number of test runs
	char test_file_path[] = "/mnt/extsd/test.bin";

	for (int i = 0; i < sizeof(sizes) / sizeof(sizes[0]); i++) {
		int size = sizes[i];
		printf("Testing with file size: %dM\n", size);

		double total_write_speed = 0;
		double total_read_speed = 0;
		double write_speeds[count];
		double read_speeds[count];

		double max_write_speed = 0, min_write_speed = 1e9;
		double max_read_speed = 0, min_read_speed = 1e9;

		for (int j = 0; j < count; j++) {
			printf("Test run #%d...\n", j + 1);

			// Convert size to bytes
			size_t bytes = size * 1024 * 1024;

			// Perform write test (direct write)
			double write_speed = write_test(test_file_path, bytes);
			write_speeds[j] = write_speed;
			total_write_speed += write_speed;

			// Update max and min write speed
			if (write_speed > max_write_speed) max_write_speed = write_speed;
			if (write_speed < min_write_speed) min_write_speed = write_speed;

			drop_caches();

			// Perform read test (direct read)
			double read_speed = read_test(test_file_path, bytes);
			read_speeds[j] = read_speed;
			total_read_speed += read_speed;

			// Update max and min read speed
			if (read_speed > max_read_speed) max_read_speed = read_speed;
			if (read_speed < min_read_speed) min_read_speed = read_speed;

			// Remove the test file after each run
			remove(test_file_path);
		}

		// Calculate and output average write and read speeds
		double avg_write_speed = total_write_speed / count;
		double avg_read_speed = total_read_speed / count;

		// Output the header for the table
		printf("\n| Test Run |");
		for (int j = 0; j < count; j++) {
			printf(" Run #%d   |", j + 1);
		}
		printf("\n");

		// Output the divider line
		printf("|----------|");
		for (int j = 0; j < count; j++) {
			printf("----------|");
		}
		printf("\n");

		// Output the write speeds
		printf("| Write    |");
		for (int j = 0; j < count; j++) {
			printf(" %-8.2f |", write_speeds[j]);
		}
		printf("\n");

		// Output the read speeds
		printf("| Read     |");
		for (int j = 0; j < count; j++) {
			printf(" %-8.2f |", read_speeds[j]);
		}
		printf("\n");

		// Output maximum, minimum, and average speeds
		printf("Max write speed: %.2f MB/s\n", max_write_speed);
		printf("Min write speed: %.2f MB/s\n", min_write_speed);
		printf("Avg write speed: %.2f MB/s\n", avg_write_speed);

		printf("Max read speed: %.2f MB/s\n", max_read_speed);
		printf("Min read speed: %.2f MB/s\n", min_read_speed);
		printf("Avg read speed: %.2f MB/s\n\n", avg_read_speed);
	}

	return 0;
}
