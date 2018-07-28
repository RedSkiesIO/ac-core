STDFLAGS:=-std=c++17
PREFIX:=/usr/local
DEBUGFLAGS:=-g -O0 ${STDFLAGS} -DDEBUG
RELEASEFLAGS:=-O3 ${STDFLAGS}

#dev
CXXFLAGS:=-g -O0 -DDEBUG ${STDFLAGS}
#distr
#CXXFLAGS:=${STDFLAGS}
export CXXFLAGS

debug: export CXXFLAGS:=${DEBUGFLAGS}
debug: all

release: export CXXFLAGS:=${RELEASEFLAGS}
release: all

cryptos-debug: export CXXFLAGS:=${DEBUGFLAGS} -DCRYPTOS
 cryptos-debug: all

cryptos-release: export CXXFLAGS:=${RELEASEFLAGS} -DCRYPTOS
cryptos-release: all

all: gov/libusgov.so wallet/libuswallet.so govx/us-gov walletx/us-wallet

wallet-debug: export CXXFLAGS:=${DEBUGFLAGS}
wallet-debug: wallet

wallet-release: export CXXFLAGS:=${RELEASEFLAGS}
wallet-release: wallet

gov: govx/us-gov
wallet: walletx/us-wallet

install: release
	install gov/libusgov.so ${PREFIX}/lib
	install wallet/libuswallet.so ${PREFIX}/lib
	install govx/us-gov ${PREFIX}/bin
	install walletx/us-wallet ${PREFIX}/bin

ifeq ($(FCGI),1)
	cat etc/init.d/us-wallet | sed "s/\(^DAEMON_ARGS=\".*\)\" *#A; INSTALLER.*/\1 -fcgi -json \"/" >/tmp/usgif
	cat /tmp/usgif | sed "s@^\(DAEMON=\"\).*\" *#D; INSTALLER.*@\1/usr/bin/spawn-fcgi -p 9000 -n /usr/local/bin/us-wallet -- \"@" >/tmp/us-wallet
	install /tmp/us-wallet /etc/init.d/
	rm /tmp/usgif
	rm /tmp/us-wallet
else
	install etc/init.d/us-wallet /etc/init.d/
endif

	install etc/init.d/us-gov /etc/init.d/
	ldconfig
	systemctl daemon-reload

cryptos-install: cryptos-release
	install gov/libusgov.so ${PREFIX}/lib
	install wallet/libuswallet.so ${PREFIX}/lib
	install govx/us-gov ${PREFIX}/bin
	install walletx/us-wallet ${PREFIX}/bin
	install cryptos/etc/init.d/us-wallet /etc/init.d/
	install cryptos/etc/init.d/us-gov /etc/init.d/
	ldconfig /usr/local/lib

install-nginx:
	install etc/nginx/sites_available/us-wallet-api.conf /etc/nginx/sites_available
	install etc/nginx/snippets/snakeoil.conf /etc/nginx/snippets/
	install etc/ssl/certs/ssl-cert-snakeoil.pem /etc/ssl/certs/
	install etc/ssl/private/ssl-cert-snakeoil.key /etc/ssl/private/
	install var/www/html/index.html /var/www/html/

gov/libusgov.so: api/apitool_generated_*
	$(MAKE) CXXFLAGS="${CXXFLAGS} -fPIC" -C gov;

govx/us-gov: gov/libusgov.so
	$(MAKE) CXXFLAGS="${CXXFLAGS}" -C govx;


wallet/libuswallet.so: gov/libusgov.so api/apitool_generated_*
	$(MAKE) CXXFLAGS="${CXXFLAGS} -fPIC" -C wallet;

walletx/us-wallet: wallet/libuswallet.so
ifeq ($(FCGI),1)
	$(MAKE) CXXFLAGS="${CXXFLAGS}" FCGI=1 -C walletx ;
else
	$(MAKE) CXXFLAGS="${CXXFLAGS}" -C walletx ;
endif

api/apitool_generated_*: apitool/apitool
	$(MAKE) CXXFLAGS="${CXXFLAGS}" -C api ;

apitool/apitool:
	$(MAKE) CXXFLAGS="${CXXFLAGS}" -C apitool ;

.PHONY: all
.PHONY: wallet
.PHONY: gov
.PHONY: debug
.PHONY: release

clean:
	$(MAKE) clean -C gov; \
	$(MAKE) clean -C govx; \
	$(MAKE) clean -C wallet; \
	$(MAKE) clean -C walletx;
