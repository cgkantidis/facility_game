public class FServer {
	int port; // TCP port for accepting the socket connection
	FPlayer serverPlayer; // The player object for FServer
	EnumPlayerType playerType;

	// Determine if server is player A or player B
	boolean serverIsPlayerA;

	// Player A or B
	EnumPlayer serverRole;
	EnumPlayer clientRole;

	FacilityGame game;
	ObjectInputStream in;
	ObjectOutputStream out;

	boolean verbose; // print information during the execution

	// A thread to monitor the player state
	MonitorThread monitorThread;

	// Server Socket for listen for accepting client connection
	ServerSocket ss;

	// Socket for the connection with the client
	Socket s;

	public FServer(boolean parVerbose, int parPort, EnumPlayerType playerType) {
		verbose = parVerbose;
		port = parPort;
		this.playerType = playerType;
		monitorThread = new MonitorThread();
	}

	public void printGameStatus() {
		if (verbose) {
			game.printStatusLong();
			game.printAllMoves();
		} else {
			game.printStatus();
		}
	}

	public void printGameInfo() {
		game.printGameInfo();
	}

	public Socket acceptConnection() {
		System.out.println("FServer: FacilityServer " + Finals.VERSION);
		System.out.println("FServer: FacilityServer is starting ...");

		ss = null;
		s = null;

		try {
			ss = new ServerSocket(port);
			InetAddress addr = InetAddress.getLocalHost();

			System.out.println("FServer: The server is listening at " + addr
					+ " on port: " + port
					+ " and waiting for an FClient to connect ...");

			s = ss.accept();

		} catch (UnknownHostException e) {
			System.err.println(e);
			System.exit(-1);
			// e.printStackTrace();
		} catch (BindException e) {
			System.err.println(e);
			System.err
					.println("Is there another instance of this server running?");
			System.exit(-1);
			// e.printStackTrace();
		} catch (IOException e) {
			System.err.println(e);
			System.exit(-1);
			// e.printStackTrace();
		}

		return s;
	}

	public void serve(Socket socket) throws FacilityGameException {
		try {
			s = socket;
			InetAddress clientAddress = s.getInetAddress();
			System.out.println("FServer: Established connection with "
					+ clientAddress);

			in = new ObjectInputStream(new BufferedInputStream(
					s.getInputStream()));

			System.out
					.println("FServer: The server is waiting to receive the game parameters from the client ...");

			String msgFromClient = in.readUTF();
			System.out.println("Msg from Client: " + msgFromClient);

			out = new ObjectOutputStream(new BufferedOutputStream(
					s.getOutputStream()));

			out.writeUTF("OK from FServer " + Finals.VERSION);
			out.writeBoolean(true); // Send true (meaning OK)
			out.flush();

			int n = in.readInt();
			long seed = in.readLong();
			EnumGameType gameType = (EnumGameType) in.readObject();
			boolean clientIsPlayerB = in.readBoolean();

			serverIsPlayerA = clientIsPlayerB;
			serverRole = (serverIsPlayerA ? EnumPlayer.PLAYER_A
					: EnumPlayer.PLAYER_B);
			clientRole = (serverIsPlayerA ? EnumPlayer.PLAYER_B
					: EnumPlayer.PLAYER_A);

			// check parameters
			try {
				FacilityGame.checkParameters(true, playerType, n, gameType,
						clientRole);
				game = new FacilityGame(EnumClientServer.SERVER, n, seed,
						gameType);
			} catch (FacilityGameException e) {
				out.writeUTF(e.getMessage());
				out.flush();
				throw e; // rethrow the exception
			}

			out.writeUTF("OK");
			out.flush();

			System.out
					.println("FServer: FacilityServer created local game instance with parameters: size:="
							+ n
							+ " , seed:="
							+ seed
							+ ", GameType:= "
							+ gameType + ", FServer role:= " + serverRole);

		} catch (UnknownHostException e) {
			System.err.println(e);
			System.exit(-1);
			// e.printStackTrace();
		} catch (BindException e) {
			System.err.println(e);
			System.err
					.println("Is there another instance of this server running?");
			System.exit(-1);
			// e.printStackTrace();
		} catch (IOException e) {
			System.err.println(e);
			System.exit(-1);
			// e.printStackTrace();
		} catch (ClassNotFoundException e) {
			System.err.println(e);
		}
	}

	public void play(FPlayer parServerPlayer) {
		try {
			serverPlayer = parServerPlayer;

			// Initialize the player
			serverPlayer.initialize(game);

			out.writeUTF(serverPlayer.about());
			out.flush();

			monitorThread.setState(game.getCurMoveIndex(),
					EnumPlayerState.WAITING_FOR_OPPONENT);
			monitorThread.start();

			String aboutOpponent = in.readUTF();

			if (serverIsPlayerA) {
				game.setAboutPlayerA(serverPlayer.about());
				game.setAboutPlayerB(aboutOpponent);
			} else {
				game.setAboutPlayerA(aboutOpponent);
				game.setAboutPlayerB(serverPlayer.about());
			}

			int afm = in.readInt();
			String firstname = in.readUTF();
			String lastname = in.readUTF();

			// No need to check user data in the local server
			//
			// if (afm < 50000 || firstname.equals("John")
			// || lastname.equals("Doe") || firstname.equals("")
			// || lastname.equals("")) {
			// out.writeBoolean(false);
			// out.flush();
			// System.err.println("\nWrong Personal Informations!!!\n");
			// //monitorThread.stop();
			// monitorThread.setState( game.getCurMoveIndex(),
			// EnumPlayerState.TERMINATING);
			// } else {
			out.writeBoolean(true);
			out.flush();

			System.out.println("FServer (" + serverRole
					+ "): OK, the game is starting ...");

			// Play the game
			int countMoveServer = 0;
			int countMoveClient = 0;

			if (serverIsPlayerA) {
				while (!game.isFinished()) {
					// playerA
					int moveOfServer = serverPlayer.nextMove(game);

					monitorThread.setState(game.getCurMoveIndex(),
							EnumPlayerState.WAITING_FOR_ME);
					game.processMove(moveOfServer, serverRole);
					if (verbose) {
						System.out.println("FServer: move:" + countMoveServer
								+ ", location of Server:" + moveOfServer
								+ ", sending move");
					}
					countMoveServer++;
					out.writeInt(moveOfServer);
					out.flush();

					if (game.isFinished()) {
						continue;
					}

					if (verbose) {
						System.out.println("FServer: Receiving move of Client");
					}
					monitorThread.setState(game.getCurMoveIndex(),
							EnumPlayerState.WAITING_FOR_OPPONENT);
					int moveOfClient = in.readInt();

					game.processMove(moveOfClient, clientRole);
					if (verbose) {
						System.out.println("FServer: move of Client:"
								+ countMoveClient + ", location of Client:"
								+ moveOfClient);
					}
					countMoveClient++;
				}
			} else {
				// Server is Player B
				while (!game.isFinished()) {

					if (verbose) {
						System.out.println("FServer: Receiving move of Client");
					}
					monitorThread.setState(game.getCurMoveIndex(),
							EnumPlayerState.WAITING_FOR_OPPONENT);
					int moveOfClient = in.readInt();
					game.processMove(moveOfClient, clientRole);
					if (verbose) {
						System.out.println("FServer: move of Client:"
								+ countMoveClient + ", location of Client:"
								+ moveOfClient);
					}
					countMoveClient++;

					if (game.isFinished()) {
						continue;
					}

					// playerA
					int moveOfServer = serverPlayer.nextMove(game);

					monitorThread.setState(game.getCurMoveIndex(),
							EnumPlayerState.WAITING_FOR_ME);
					game.processMove(moveOfServer, serverRole);
					if (verbose) {
						System.out.println("FServer: move:" + countMoveServer
								+ ", location of Server:" + moveOfServer
								+ ", sending move");
					}
					countMoveServer++;
					out.writeInt(moveOfServer);
					out.flush();

				}
			}

			monitorThread.setState(game.getCurMoveIndex(),
					EnumPlayerState.TERMINATING);

			System.out.println("FServer (" + serverRole
					+ "): The game finished !!");
			System.out.println("FServer (" + serverRole + "): "
					+ game.getScore());

			String proof = UUID.randomUUID().toString();
			out.writeUTF(proof);
			out.flush();
			// }

			in.close();
			out.close();

		} catch (IOException e) {
			System.err.println(e);
			e.printStackTrace();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public void printRoles() {
		System.out.println("FClient: " + clientRole);
		System.out.println("FServer: " + serverRole);
	}

	public static void main(String[] args) {
		EnumPlayerType playerType;
		boolean verbose;
		int port;

		System.out
				.println("FServer: Usage"
						+ "java -cp <jarfile.jar> facilityGame.FServer <player_type> <verbose> <port>");

		// Player type
		if (args.length >= 1) {
			playerType = EnumPlayerType.valueOf(args[0]);
		} else {
			// default parameter values
			playerType = Finals.DEFAULT_PLAYER_TYPE;
		}

		// Verbose mode
		if (args.length >= 2) {
			verbose = Boolean.parseBoolean(args[1]);
		} else {
			// default parameter values
			verbose = Finals.DEFAULT_VERBOSE_MODE;
		}

		// Listener port
		if (args.length >= 3) {
			port = Integer.parseInt(args[2]);
		} else {
			// default parameter values
			port = Finals.DEFAULT_PORT;
		}

		// Create FServer object
		FServer server = new FServer(verbose, port, playerType);

		// Wait for client to connect
		Socket s = server.acceptConnection();

		try {
			// Serve the client request
			server.serve(s);

			// Create player object for PlayerA
			FPlayer serverPlayer = CreatePlayerObject.create(server.serverRole,
					playerType);

			// Print the player type and player info
			System.out.println("FServer: " + playerType + ", "
					+ serverPlayer.about());

			// Play the game
			server.play(serverPlayer);

			// Print the game info
			server.printGameInfo();

			// Print the game status
			server.printGameStatus();

			// print roles
			server.printRoles();
		} catch (FacilityGameException e) {
			System.err.println(e);

		}
	}
}

