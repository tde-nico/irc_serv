#include "DBConn.hpp"

#include <time.h>

int	main()
{
	DBConn	*db;

	// Connect
	db = new DBConn("127.0.0.1", "5432", "postgres", "postgres", "test");
	if (db->status == 1)
		return (1);


	#if 0 // Insert
		time_t		raw_time;
		struct tm	*time_info;
		char		buffer[80];
		time(&raw_time);
		time_info = localtime(&raw_time);
		strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", time_info);
		std::string now(buffer);
		std::cout << now << "\n";
		PGresult *ress = db->exec(fmt_str("INSERT INTO logs (log, date) VALUES ('%s', '%s')", "2", now.c_str()));

		// Check the number of affected rows (optional)
		int rowsAffected = std::stoi(PQcmdTuples(ress));
		std::cout << "Rows affected: " << rowsAffected << "\n\n";

		// Clean up
		PQclear(ress);
	#endif


	#if 0 // Delete
		PGresult *ress = db->exec(fmt_str("DELETE FROM logs WHERE id = %s;", "2"));

		// Check the number of affected rows (optional)
		int rowsAffected = std::stoi(PQcmdTuples(ress));
		std::cout << "Rows affected: " << rowsAffected << "\n\n";

		// Clean up
		PQclear(ress);
	#endif


	#if 0 // Query
		PGresult *res = db->query("SELECT * FROM logs;");
		
		// Extract and print the data
		int numRows = PQntuples(res);
		int numCols = PQnfields(res);

		std::cout << "Number of rows: " << numRows << std::endl;
		std::cout << "Number of columns: " << numCols << std::endl;
		for (int i = 0; i < numCols; ++i) {
			std::cout << PQfname(res, i) << "\t";
		}
		std::cout << std::endl;

		// Print data
		for (int row = 0; row < numRows; ++row) {
			for (int col = 0; col < numCols; ++col) {
				std::cout << PQgetvalue(res, row, col) << "\t";
			}
			std::cout << std::endl;
		}

		// Clean up
		PQclear(res);
	#endif

	// Finish
	delete db;
	return (0);
}

// $(pkg-config --cflags --libs libpq)
// c++ -Werror -Wextra -Wall db_test.cpp DBConn.cpp -o db -I/usr/include/postgresql -lpq
