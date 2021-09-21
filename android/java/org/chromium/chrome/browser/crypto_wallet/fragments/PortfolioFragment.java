/* Copyright (c) 2021 The Brave Authors. All rights reserved.
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this file,
 * You can obtain one at http://mozilla.org/MPL/2.0/. */

package org.chromium.chrome.browser.crypto_wallet.fragments;

import android.annotation.SuppressLint;
import android.app.Activity;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.MotionEvent;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.annotation.Nullable;
import androidx.fragment.app.Fragment;
import androidx.recyclerview.widget.LinearLayoutManager;
import androidx.recyclerview.widget.RecyclerView;

import org.chromium.brave_wallet.mojom.EthJsonRpcController;
import org.chromium.chrome.R;
import org.chromium.chrome.browser.crypto_wallet.activities.BraveWalletActivity;
import org.chromium.chrome.browser.crypto_wallet.adapters.WalletCoinAdapter;
import org.chromium.chrome.browser.crypto_wallet.listeners.OnWalletListItemClick;
import org.chromium.chrome.browser.crypto_wallet.model.WalletListItemModel;
import org.chromium.chrome.browser.crypto_wallet.util.SmoothLineChartEquallySpaced;
import org.chromium.chrome.browser.crypto_wallet.util.Utils;

import java.util.ArrayList;
import java.util.List;

import org.chromium.ui.widget.Toast;

import org.chromium.brave_wallet.mojom.AccountInfo;
import org.chromium.brave_wallet.mojom.KeyringController;
import org.chromium.base.Log;
import org.chromium.chrome.browser.crypto_wallet.EthJsonRpcControllerFactory;
import org.chromium.brave_wallet.mojom.EthJsonRpcController;
import org.chromium.mojo.bindings.ConnectionErrorHandler;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.ExecutionException;

import java.util.Arrays;


public class PortfolioFragment
        extends Fragment implements OnWalletListItemClick, AdapterView.OnItemSelectedListener {
    Spinner mSpinner;
    TextView mBalance;

    public static PortfolioFragment newInstance() {
        return new PortfolioFragment();
    }

    private EthJsonRpcController getEthJsonRpcController() {
        Activity activity = getActivity();
        if (activity instanceof BraveWalletActivity) {
            return ((BraveWalletActivity) activity).getEthJsonRpcController();
        }

        return null;
    }

    @Override
    public void onCreate(@Nullable Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setHasOptionsMenu(true);
    }

    @Nullable
    @Override
    public View onCreateView(@NonNull LayoutInflater inflater, @Nullable ViewGroup container,
            @Nullable Bundle savedInstanceState) {
        View view = inflater.inflate(R.layout.fragment_portfolio, container, false);

        view.setOnTouchListener(new View.OnTouchListener() {
            @Override
            @SuppressLint("ClickableViewAccessibility")
            public boolean onTouch(View v, MotionEvent event) {
                SmoothLineChartEquallySpaced chartES = view.findViewById(R.id.line_chart);
                if (chartES == null) {
                    return true;
                }
                if (event.getAction() == MotionEvent.ACTION_MOVE
                        || event.getAction() == MotionEvent.ACTION_DOWN) {
                    chartES.drawLine(event.getRawX(), null);
                } else if (event.getAction() == MotionEvent.ACTION_UP) {
                    chartES.drawLine(-1, null);
                }

                return true;
            }
        });

        mSpinner = view.findViewById(R.id.spinner);
        mSpinner.setOnItemSelectedListener(this);

        mBalance = view.findViewById(R.id.balance);
        mBalance.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                UpdatePortfolio();
            }
        });

        // Creating adapter for spinner
        ArrayAdapter<String> dataAdapter = new ArrayAdapter<String>(getActivity(),
                android.R.layout.simple_spinner_item, Utils.getNetworksList(getActivity()));
        dataAdapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        mSpinner.setAdapter(dataAdapter);

        return view;
    }

    private void updateNetwork() {
        EthJsonRpcController ethJsonRpcController = getEthJsonRpcController();
        if (ethJsonRpcController != null) {
            ethJsonRpcController.getChainId(
                    chain_id -> { mSpinner.setSelection(getIndexOf(mSpinner, chain_id)); });
        }
    }

    @Override
    public void onResume() {
        super.onResume();
        updateNetwork();
    }

    private int getIndexOf(Spinner spinner, String chain_id) {
        String strNetwork = Utils.getNetworkText(getActivity(), chain_id).toString();
        for (int i = 0; i < spinner.getCount(); i++) {
            if (spinner.getItemAtPosition(i).toString().equalsIgnoreCase(strNetwork)) {
                return i;
            }
        }

        return 0;
    }

    @Override
    public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {
        String item = parent.getItemAtPosition(position).toString();
        EthJsonRpcController ethJsonRpcController = getEthJsonRpcController();
        if (ethJsonRpcController != null) {
            ethJsonRpcController.setNetwork(Utils.getNetworkConst(getActivity(), item));
        }
    }

    @Override
    public void onNothingSelected(AdapterView<?> arg0) {}

    @Override
    public void onViewCreated(@NonNull View view, @Nullable Bundle savedInstanceState) {
        super.onViewCreated(view, savedInstanceState);
        assert getActivity() != null;

        SmoothLineChartEquallySpaced chartES = view.findViewById(R.id.line_chart);
        chartES.setColors(new int[] {0xFFF73A1C, 0xFFBF14A2, 0xFF6F4CD2});
        chartES.setData(new float[] {15, 21, 9, 21, 25, 35, 24, 28});

        setUpCoinList(view);
        Button editVisibleAssets = view.findViewById(R.id.edit_visible_assets);
        editVisibleAssets.setOnClickListener(v -> {
            EditVisibleAssetsBottomSheetDialogFragment bottomSheetDialogFragment =
                    EditVisibleAssetsBottomSheetDialogFragment.newInstance(
                            WalletCoinAdapter.AdapterType.EDIT_VISIBLE_ASSETS_LIST);
            bottomSheetDialogFragment.show(
                    getFragmentManager(), EditVisibleAssetsBottomSheetDialogFragment.TAG_FRAGMENT);
        });
    }

    private void setUpCoinList(View view) {
        RecyclerView rvCoins = view.findViewById(R.id.rvCoins);
        WalletCoinAdapter walletCoinAdapter =
                new WalletCoinAdapter(WalletCoinAdapter.AdapterType.VISIBLE_ASSETS_LIST);
        List<WalletListItemModel> walletListItemModelList = new ArrayList<>();
        walletListItemModelList.add(new WalletListItemModel(
                R.drawable.ic_eth, "Ethereum", "ETH", "$872.48", "0.31178 ETH"));
        walletCoinAdapter.setWalletListItemModelList(walletListItemModelList);
        walletCoinAdapter.setOnWalletListItemClick(PortfolioFragment.this);
        walletCoinAdapter.setWalletListItemType(Utils.ASSET_ITEM);
        rvCoins.setAdapter(walletCoinAdapter);
        rvCoins.setLayoutManager(new LinearLayoutManager(getActivity()));
    }

    @Override
    public void onAssetClick() {
        Utils.openAssetDetailsActivity(getActivity());
    }

    private void UpdatePortfolio() {
        try {
            // For current network:
            // Get all accounts
            KeyringController keyringController = getKeyringController();
            assert keyringController != null;
            keyringController.getDefaultKeyringInfo(keyringInfo -> {
                if (keyringInfo != null) {
                    AccountInfo[] accountInfos = keyringInfo.accountInfos;
                    EthJsonRpcController rpcController =  getEthJsonRpcController();
                    ArrayList<CompletableFuture<Void>> futures = new ArrayList<>();

                    for (AccountInfo accountInfo : accountInfos) {
                        CompletableFuture<Void> future = CompletableFuture.runAsync(() -> {
                            Log.e("TAGAB", "IN FUTURE: accountInfo.address="+accountInfo.address);
                            rpcController.getBalance(accountInfo.address, (result, balance) -> {
                                    Log.e("TAG", "result="+result);
                                    Log.e("TAG", "balance="+balance);
                            });
                        });
                        futures.add(future);
                    }

                    Log.e("TAG", "Waiting for all futures to complete");

                    // OK
                    String[] stringArr = new String[10];

                    //W/System.err(4422): java.lang.ClassCastException: java.lang.Object[] cannot be cast to java.util.concurrent.CompletableFuture[]
                    CompletableFuture<Void> all = CompletableFuture.allOf((CompletableFuture<Void>[])futures.toArray());

                    // error: generic array creation
                    //CompletableFuture<Void>[] futureArr = new CompletableFuture<Void>[accountInfos.length];
                    //CompletableFuture<Void> all = CompletableFuture.allOf(futures.toArray(futureArr));

                    // error: generic array creation
                    // CompletableFuture<Void> all = CompletableFuture.allOf(
                    //     Arrays.stream(futures.toArray(CompletableFuture<Void>[]::new))
                    // );

                    try {
                        all.get();
                    } catch(ExecutionException | InterruptedException ex) {
                        Log.e("TAG", "ExecutionException ex=" + ex);
                    }
                }
            });

        } catch(Exception ex) {
            Log.e("TAG", "Exception ex="+ex);
        }

        // For each account get amount in ETH

        // Get ratings

        // Multiply and add

        // Put result
        mBalance.setText("$347");
    }

    // TODO(AlexeyBarabash): duplicate the same as AccountsFragment.getKeyringController()
    private KeyringController getKeyringController() {
        Activity activity = getActivity();
        if (activity instanceof BraveWalletActivity) {
            return ((BraveWalletActivity) activity).getKeyringController();
        }

        return null;
    }
}
